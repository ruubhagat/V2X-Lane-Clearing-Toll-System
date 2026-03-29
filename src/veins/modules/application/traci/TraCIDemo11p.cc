#include "veins/modules/application/traci/TraCIDemo11p.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

using namespace veins;

Define_Module(veins::TraCIDemo11p);

void TraCIDemo11p::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;

        nodeID = getParentModule()->getIndex();
        senderID = -1;

        isEmergencyVehicle = (nodeID == EMERGENCY_VEHICLE_NODE);
        emergencyActive = false;
        tollCleared = false;
        isInTollZone = false;
    }
}

void TraCIDemo11p::onWSA(DemoServiceAdvertisment* wsa)
{
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        currentSubscribedServiceId = wsa->getPsid();
        if (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService(static_cast<Channel>(wsa->getTargetChannel()),
                         wsa->getPsid(),
                         "Mirrored Traffic Service");
        }
    }
}

void TraCIDemo11p::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

    senderID = wsm->getSenderID();

    if (strcmp(wsm->getName(), "PRIORITY_CLEAR") == 0) {
        emergencyActive = true;
        traciVehicle->changeLane(0, 2);
        traciVehicle->setLaneChangeMode(0b0000001000);
        return;
    }

    if (strcmp(wsm->getName(), "TOLL_CLEARED") == 0) {
        tollCleared = true;
        traciVehicle->setSpeedMode(0);
        return;
    }

    if (senderID == EMERGENCY_VEHICLE_NODE && !isEmergencyVehicle) {

        findHost()->getDisplayString().setTagArg("i", 1, "yellow");

        traciVehicle->changeLane(0, 2);
        traciVehicle->setLaneChangeMode(0b0000001000);

        if (!sentMessage) {
            sentMessage = true;
            wsm->setSenderAddress(myId);
            wsm->setSerial(3);
            scheduleAt(simTime() + 20 + uniform(0.01, 0.2), wsm->dup());
        }
    }
}

void TraCIDemo11p::handleSelfMsg(cMessage* msg)
{
    if (TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(msg)) {

        sendDown(wsm->dup());
        wsm->setSerial(wsm->getSerial() + 1);

        if (wsm->getSerial() >= 3) {
            stopService();
            delete wsm;
        }
        else {
            scheduleAt(simTime() + 1, wsm);
        }
    }
    else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void TraCIDemo11p::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    if (isEmergencyVehicle && !sentMessage) {

        sentMessage = true;

        TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
        populateWSM(wsm);
        wsm->setSenderID(nodeID);
        wsm->setName("EMERGENCY");
        wsm->setSerial(3);

        scheduleAt(simTime() + 1, wsm->dup());
    }

    if (isInTollZone && !tollCleared) {

        TraCIDemo11pMessage* toll = new TraCIDemo11pMessage();
        populateWSM(toll);
        toll->setName("TOLL_REQ");
        toll->setSenderID(nodeID);
        sendDown(toll);
    }

    lastDroveAt = simTime();
}
