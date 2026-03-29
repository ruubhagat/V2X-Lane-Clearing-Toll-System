#include "veins/modules/application/traci/MyVeinsApp.h"
#include <numeric>
#include <algorithm>

using namespace veins;

Define_Module(veins::MyVeinsApp);

void MyVeinsApp::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        cModule* mobilityModule = getParentModule()->getSubmodule("veinsmobility");
        mobility = check_and_cast<TraCIMobility*>(mobilityModule);
        traci = mobility->getCommandInterface();
        traciVehicle = mobility->getVehicleCommandInterface();

        operationMode = par("operationMode").intValue();
        isMalicious = par("isMalicious").boolValue();
        attackProbability = par("attackProbability").doubleValue();
        
        // Init New Metrics
        departureTime = simTime();
        totalDistance = 0.0;
        lastPosition = Coord::ZERO;
        beaconsSent = 0;
        beaconsReceived = 0;
        emergencyDetected = false;
        delayIncurred = 0.0;
        spoofingAttempts = 0;
        totalResponseTime = 0;
        laneChangesPerformed = 0;
        emergencyMessagesReceived = 0;

        std::string vehType = traciVehicle->getTypeId();
        
        if (vehType.find("ambulance") != std::string::npos || vehType.find("emergency") != std::string::npos) {
            isEmergencyVehicle = true;
            myPsid = 1; 
            traciVehicle->setColor(TraCIColor(255, 0, 0, 255));
            traciVehicle->setMaxSpeed(25.0);
            
            sendEmergencyBeacon = new cMessage("sendEmergencyBeacon");
            scheduleAt(simTime() + 0.1, sendEmergencyBeacon);
            checkEmergencyTimeout = nullptr;
            maliciousAttackTimer = nullptr;
        } else {
            isEmergencyVehicle = false;
            myPsid = 0; 
            traciVehicle->setColor(TraCIColor(0, 0, 255, 255));
            
            sendEmergencyBeacon = nullptr;
            checkEmergencyTimeout = new cMessage("checkEmergencyTimeout");
            scheduleAt(simTime() + 1.0, checkEmergencyTimeout);

            if (isMalicious) {
                traciVehicle->setColor(TraCIColor(0, 0, 0, 255));
                maliciousAttackTimer = new cMessage("maliciousAttack");
                scheduleAt(simTime() + uniform(2.0, 5.0), maliciousAttackTimer);
            } else {
                maliciousAttackTimer = nullptr;
            }
        }

        emergencyVehicleNearby = false;
        isChangingLane = false;
        laneFormationComplete = false;
        currentLane = 0;
        targetLane = 0;
        beaconInterval = 0.5;
        emergencyTimeout = 3.0;
    }
    else if (stage == 1) {
        currentLane = traciVehicle->getLaneIndex();
    }
}

void MyVeinsApp::handlePositionUpdate(cObject* obj) {
    DemoBaseApplLayer::handlePositionUpdate(obj);
    
    // TRACK DISTANCE
    Coord currentPos = mobility->getPositionAt(simTime());
    if (lastPosition != Coord::ZERO) {
        totalDistance += currentPos.distance(lastPosition);
    }
    lastPosition = currentPos;

    if (!isEmergencyVehicle && isChangingLane) {
        if (traciVehicle->getLaneIndex() == targetLane) {
            isChangingLane = false;
            laneFormationComplete = true;
            traciVehicle->setColor(TraCIColor(0, 0, 255, 255)); 
        }
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
    } else if (msg == maliciousAttackTimer) {
        if (uniform(0, 1) < attackProbability) {
            sendSpoofedEmergencyBeacon();
        }
        scheduleAt(simTime() + uniform(2.0, 5.0), maliciousAttackTimer);
    } else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void MyVeinsApp::sendEmergencyMessage() {
    beaconsSent++;
    BaseFrame1609_4* wsm = new BaseFrame1609_4("EMERGENCY");
    populateWSM(wsm);
    wsm->setPsid(myPsid); 
    wsm->setRecipientAddress(-1); 
    sendDown(wsm);
}

void MyVeinsApp::sendSpoofedEmergencyBeacon() {
    spoofingAttempts++;
    beaconsSent++;
    BaseFrame1609_4* wsm = new BaseFrame1609_4("EMERGENCY");
    populateWSM(wsm);
    wsm->setPsid(1); 
    wsm->setRecipientAddress(-1); 
    sendDown(wsm);
}

void MyVeinsApp::onWSM(BaseFrame1609_4* wsm) {
    beaconsReceived++;
    double latency = (simTime() - wsm->getTimestamp()).dbl() * 1000.0;
    latencies.push_back(latency);

    if (strcmp(wsm->getName(), "EMERGENCY") == 0) handleEmergencyMessage(wsm);
}

void MyVeinsApp::handleEmergencyMessage(BaseFrame1609_4* wsm) {
    if (isEmergencyVehicle) return; 

    if (operationMode == 1 && uniform(0, 1) > 0.2) return; 

    if (!emergencyDetected) {
        emergencyDetected = true;
        emergencyDetectionTime = simTime();
    }

    if (!emergencyVehicleNearby) {
        emergencyVehicleNearby = true;
        lastEmergencyBeacon = simTime();
        emergencyMessagesReceived++;
        clearPathForEmergency();
    } else {
        lastEmergencyBeacon = simTime();
        totalResponseTime += (simTime() - emergencyDetectionTime);
    }
}

void MyVeinsApp::clearPathForEmergency() {
    if (isChangingLane || laneFormationComplete) return;
    currentLane = traciVehicle->getLaneIndex();
    
    if (currentLane > 0) {
        targetLane = 0; 
        performLaneChange();
    } else {
        traciVehicle->slowDown(traciVehicle->getSpeed() * 0.5, 2000); 
        laneFormationComplete = true;
    }
}

void MyVeinsApp::performLaneChange() {
    try {
        traciVehicle->changeLane(targetLane, 2000); 
        isChangingLane = true;
        laneChangesPerformed++;
        laneChangeTime = simTime();
        if (emergencyDetected) {
            delayIncurred = (laneChangeTime - emergencyDetectionTime).dbl();
        }
        traciVehicle->setColor(TraCIColor(255, 255, 0, 255)); 
    } catch (...) {
        laneFormationComplete = true; 
    }
}

void MyVeinsApp::resumeNormalDriving() {
    emergencyVehicleNearby = false;
    isChangingLane = false;
    laneFormationComplete = false;
    traciVehicle->setSpeed(-1);
    traciVehicle->setColor(TraCIColor(0, 0, 255, 255)); 
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

        EV << "\n=============================================" << endl;
        EV << "   EMERGENCY VEHICLE " << myId << " REPORT" << endl;
        EV << "=============================================" << endl;
        EV << " Travel Time    : " << travelTime << " seconds" << endl;
        EV << " Distance       : " << totalDistance << " meters" << endl;
        EV << " Average Speed  : " << avgSpeed << " m/s" << endl;
        EV << " Beacons Sent   : " << beaconsSent << endl;
        EV << "=============================================\n" << endl;
    } else {
        recordScalar("emergencyDetected", emergencyDetected ? 1 : 0);
        recordScalar("laneChangesPerformed", laneChangesPerformed);
        if (emergencyDetected) recordScalar("detectionLatency", delayIncurred);
        
        recordScalar("beaconsReceived", beaconsReceived);
        recordScalar("beaconsSent", beaconsSent);
        
        if (!latencies.empty()) {
            double avgLatency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
            double minLatency = *std::min_element(latencies.begin(), latencies.end());
            double maxLatency = *std::max_element(latencies.begin(), latencies.end());
            
            recordScalar("avgLatency", avgLatency);
            recordScalar("minLatency", minLatency);
            recordScalar("maxLatency", maxLatency);
        }
        
        if (beaconsSent > 0) {
            double pdr = (double)beaconsReceived / beaconsSent * 100.0;
            recordScalar("packetDeliveryRatio", pdr);
        }
        
        if (isMalicious) recordScalar("spoofingAttempts", spoofingAttempts);
    }

    if (sendEmergencyBeacon) cancelAndDelete(sendEmergencyBeacon);
    if (checkEmergencyTimeout) cancelAndDelete(checkEmergencyTimeout);
    if (maliciousAttackTimer) cancelAndDelete(maliciousAttackTimer);
}

void MyVeinsApp::onBSM(DemoSafetyMessage* bsm) {}
void MyVeinsApp::onWSA(DemoServiceAdvertisment* wsa) {}
