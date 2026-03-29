#pragma once
#include "veins/veins.h"
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include <vector>
#include <map>

namespace veins {

class VEINS_API MyVeinsApp : public DemoBaseApplLayer {
public:
    virtual void initialize(int stage) override;
    virtual void finish() override;

protected:
    virtual void onBSM(DemoSafetyMessage* bsm) override;
    virtual void onWSM(BaseFrame1609_4* wsm) override;
    virtual void onWSA(DemoServiceAdvertisment* wsa) override;
    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void handlePositionUpdate(cObject* obj) override;

    void sendEmergencyMessage();
    void handleEmergencyMessage(DemoSafetyMessage* bsm);
    void clearPathForEmergency();
    void performLaneChange();
    void resumeNormalDriving();

    TraCIMobility* mobility;
    TraCICommandInterface* traci;
    TraCICommandInterface::Vehicle* traciVehicle;

    bool isEmergencyVehicle;
    bool emergencyVehicleNearby;

    // API FIX: Uses myPsid
    int  myPsid;

    bool isChangingLane;
    bool laneFormationComplete;
    int  currentLane;
    int  targetLane;

    simtime_t departureTime;
    simtime_t arrivalTime;
    double    totalDistance;
    bool      lastPositionValid;
    Coord     lastPosition;

    int  beaconsSent;
    int  beaconsReceived;
    std::vector<double> latencies;

    bool      emergencyDetected;
    simtime_t emergencyDetectionTime;
    double    delayIncurred;
    simtime_t lastEmergencyBeacon;
    int       laneChangesPerformed;

    double beaconInterval;
    double emergencyTimeout;

    cMessage* sendEmergencyBeacon;
    cMessage* checkEmergencyTimeout;

    int    operationMode;
    double baseTollVal;
    double hLatencyVal;
    double reactionJitterVal;
    double totalRevenue;

    bool   isMalicious;
    double attackProbability;
    int    spoofingAttempts;
    int    suspiciousBeacons;
    std::map<int, simtime_t> activeEmergencies;

    int    emergencyMessagesReceived;
};

}
