#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"

namespace veins {

class VEINS_API TraCIDemo11p : public DemoBaseApplLayer {
public:
    virtual void initialize(int stage) override;

    // ----------------------------
    // Node identity
    // ----------------------------
    int nodeID;
    int senderID;

    // Emergency vehicle ID (fixed for demo)
    int EMERGENCY_VEHICLE_NODE = 5;

protected:
    // ----------------------------
    // Existing demo state
    // ----------------------------
    simtime_t lastDroveAt;
    bool sentMessage;
    int currentSubscribedServiceId;

    // ----------------------------
    // V2X / RSU extensions (NEW)
    // ----------------------------
    bool isEmergencyVehicle;      // true only for emergency node
    bool emergencyActive;         // emergency currently detected
    bool tollCleared;             // toll permission received
    bool isInTollZone;             // simple logical flag

protected:
    // ----------------------------
    // Veins callbacks
    // ----------------------------
    virtual void onWSM(BaseFrame1609_4* wsm) override;
    virtual void onWSA(DemoServiceAdvertisment* wsa) override;

    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void handlePositionUpdate(cObject* obj) override;
};

} // namespace veins
