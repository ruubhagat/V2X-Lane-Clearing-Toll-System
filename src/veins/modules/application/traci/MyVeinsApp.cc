#include "veins/modules/application/traci/MyVeinsApp.h"
#include <numeric>
#include <algorithm>

using namespace veins;
Define_Module(veins::MyVeinsApp);

void MyVeinsApp::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        cModule* mobilityModule = getParentModule()->getSubmodule("veinsmobility");
        mobility     = check_and_cast<TraCIMobility*>(mobilityModule);
        traci        = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();

        operationMode     = par("operationMode").intValue();
        isMalicious       = par("isMalicious").boolValue();
        attackProbability = par("attackProbability").doubleValue();
        baseTollVal       = par("baseToll").doubleValue();
        hLatencyVal       = par("hardwareLatency").doubleValue();
        reactionJitterVal = par("driverReactionJitter").doubleValue();

        departureTime     = simTime();
        totalDistance     = 0.0;
        lastPositionValid = false;
        lastPosition      = Coord(0, 0, 0);

        beaconsSent               = 0;
        beaconsReceived           = 0;
        emergencyDetected         = false;
        delayIncurred             = 0.0;
        spoofingAttempts          = 0;
        laneChangesPerformed      = 0;
        emergencyMessagesReceived = 0;
        suspiciousBeacons         = 0;
        totalRevenue              = 0.0;

        std::string vehType = traciVehicle->getTypeId();

        if (vehType.find("ambulance") != std::string::npos || vehType.find("emergency") != std::string::npos) {
            isEmergencyVehicle = true;
            myPsid = 1; // API FIX
            traciVehicle->setColor(TraCIColor(255, 0, 0, 255));
            traciVehicle->setMaxSpeed(25.0);

            sendEmergencyBeacon = new cMessage("sendEmergencyBeacon");
            scheduleAt(simTime() + 0.1, sendEmergencyBeacon);
            checkEmergencyTimeout = nullptr;
        } else {
            isEmergencyVehicle = false;
            myPsid = 0; // API FIX
            traciVehicle->setColor(TraCIColor(0, 0, 255, 255));

            sendEmergencyBeacon = nullptr;
            checkEmergencyTimeout = new cMessage("checkEmergencyTimeout");
            scheduleAt(simTime() + 1.0, checkEmergencyTimeout);
        }

        emergencyVehicleNearby = false;
        isChangingLane         = false;
        laneFormationComplete  = false;
        currentLane            = 0;
        targetLane             = 0;
        beaconInterval         = 1.0;
        emergencyTimeout       = 3.0;
    }
    else if (stage == 1) {
        currentLane = traciVehicle->getLaneIndex();
    }
}

void MyVeinsApp::onBSM(DemoSafetyMessage* bsm) {
    if (bsm->getPsid() == 1) { // API FIX
        beaconsReceived++;
        scheduleAt(simTime() + SimTime(hLatencyVal), new cMessage("processDelay"));
        handleEmergencyMessage(bsm);
    }
    else if (bsm->getPsid() == 0 && !isEmergencyVehicle) { // API FIX
        double surcharge = (beaconsReceived > 20) ? 0.15 : 0.0;
        totalRevenue += (baseTollVal + surcharge);
    }
}

void MyVeinsApp::clearPathForEmergency() {
    if (isChangingLane || laneFormationComplete) return;

    simtime_t jitter = SimTime(omnetpp::uniform(getRNG(0), 0.01, reactionJitterVal));

    currentLane = traciVehicle->getLaneIndex();

    if (currentLane > 0) {
        targetLane = 0;
        scheduleAt(simTime() + jitter, new cMessage("laneChangeJitter"));
        performLaneChange();
    } else {
        traciVehicle->slowDown(5.0, 2000);
        traciVehicle->setColor(TraCIColor(255, 165, 0, 255));
        laneFormationComplete = true;
        laneChangesPerformed++;
    }
}

void MyVeinsApp::performLaneChange() {
    try {
        traciVehicle->changeLane(targetLane, 2000.0);
        isChangingLane       = true;
        laneChangesPerformed++;
        traciVehicle->setColor(TraCIColor(255, 255, 0, 255));
        if (emergencyDetected) {
            delayIncurred = (simTime() - emergencyDetectionTime).dbl();
        }
    }
    catch (...) {
        traciVehicle->slowDown(5.0, 2000);
        laneFormationComplete = true;
        laneChangesPerformed++;
        if (emergencyDetected) {
            delayIncurred = (simTime() - emergencyDetectionTime).dbl();
        }
    }
}

void MyVeinsApp::resumeNormalDriving() {
    emergencyVehicleNearby = false;
    isChangingLane         = false;
    laneFormationComplete  = false;
    activeEmergencies.clear();
    traciVehicle->setSpeed(-1);
    traciVehicle->setColor(TraCIColor(0, 0, 255, 255));
}

void MyVeinsApp::handlePositionUpdate(cObject* obj) {
    DemoBaseApplLayer::handlePositionUpdate(obj);

    // API FIX
    Coord currentPos = mobility->getPositionAt(simTime());

    if (lastPositionValid) {
        totalDistance += currentPos.distance(lastPosition);
    }
    lastPosition      = currentPos;
    lastPositionValid = true;

    if (!isEmergencyVehicle && isChangingLane) {
        if (traciVehicle->getLaneIndex() == targetLane) {
            isChangingLane        = false;
            laneFormationComplete = true;
            traciVehicle->setColor(TraCIColor(0, 0, 255, 255));
        }
    }
}

void MyVeinsApp::finish() {
    DemoBaseApplLayer::finish();
    arrivalTime = simTime();

    if (isEmergencyVehicle) {
        double travelTime = (arrivalTime - departureTime).dbl();
        double avgSpeed = (travelTime > 0) ? totalDistance / travelTime : 0.0;

        recordScalar("emergencyTravelTime", travelTime);
        recordScalar("emergencyDistance", totalDistance);
        recordScalar("emergencyAvgSpeed", avgSpeed);
        recordScalar("emergencyBeaconsSent", beaconsSent);
    } else {
        recordScalar("emergencyDetected", emergencyDetected ? 1 : 0);
        recordScalar("laneChangesPerformed", laneChangesPerformed);
        recordScalar("totalRevenueGenerated", totalRevenue);
        if (emergencyDetected) recordScalar("detectionLatency", delayIncurred);
    }
}

void MyVeinsApp::handleSelfMsg(cMessage* msg) {
    if (msg == sendEmergencyBeacon) {
        sendEmergencyMessage();
        scheduleAt(simTime() + beaconInterval, sendEmergencyBeacon);
    } else if (msg == checkEmergencyTimeout) {
        if (emergencyVehicleNearby && (simTime() - lastEmergencyBeacon > emergencyTimeout)) {
            resumeNormalDriving();
        }
        scheduleAt(simTime() + 0.5, checkEmergencyTimeout);
    }
    // CRASH FIX: Deletes our custom timers before Veins can trip over them
    else if (msg->isName("processDelay") || msg->isName("laneChangeJitter")) {
        delete msg;
    } else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void MyVeinsApp::sendEmergencyMessage() {
    beaconsSent++;
    DemoSafetyMessage* bsm = new DemoSafetyMessage("EMERGENCY");
    populateWSM(bsm);
    bsm->setPsid(myPsid); // API FIX
    bsm->setSenderPos(mobility->getPositionAt(simTime())); // API FIX
    sendDown(bsm);
}

void MyVeinsApp::handleEmergencyMessage(DemoSafetyMessage* bsm) {
    if (isEmergencyVehicle || operationMode == 1) return;

    if (!emergencyDetected) {
        emergencyDetected = true;
        emergencyDetectionTime = simTime();
    }

    if (!emergencyVehicleNearby) {
        emergencyVehicleNearby = true;
        lastEmergencyBeacon = simTime();
        clearPathForEmergency();
    } else {
        lastEmergencyBeacon = simTime();
    }
}

void MyVeinsApp::onWSM(BaseFrame1609_4* wsm) {}
void MyVeinsApp::onWSA(DemoServiceAdvertisment* wsa) {}
