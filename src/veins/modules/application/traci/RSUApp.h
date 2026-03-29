#pragma once
#include "veins/veins.h"
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include <vector>

namespace veins { // <--- ADDED NAMESPACE

class VEINS_API RSUApp : public DemoBaseApplLayer {
public:
    virtual void initialize(int stage) override;
    virtual void finish() override;

protected:
    virtual void onBSM(DemoSafetyMessage* bsm) override;
    virtual void onWSM(BaseFrame1609_4* wsm) override;
    virtual void handleSelfMsg(cMessage* msg) override;

    double calculateTollAmount(bool& isPeak);

    int    dynamicTotalVehicles;
    int    dynamicEmergencyExemptions;
    double dynamicRevenue;
    int    peakHourContacts;
    int    falseExemptions;

    std::vector<double> transactionTimes;
};

} // <--- END NAMESPACE
