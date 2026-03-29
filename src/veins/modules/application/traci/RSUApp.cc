#include "veins/modules/application/traci/RSUApp.h"
#include <numeric>

namespace veins { // <--- ADDED NAMESPACE

Define_Module(RSUApp);

void RSUApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        dynamicTotalVehicles       = 0;
        dynamicEmergencyExemptions = 0;
        dynamicRevenue             = 0.0;
        peakHourContacts           = 0;
        falseExemptions            = 0;
    }
}

double RSUApp::calculateTollAmount(bool& isPeak)
{
    double baseRate = 2.50;
    int hour = (int)(simTime().dbl() / 3600.0) % 24;

    if ((hour >= 7 && hour <= 9) || (hour >= 17 && hour <= 19)) {
        isPeak = true;
        return baseRate * 1.5;
    }
    isPeak = false;
    return baseRate;
}

void RSUApp::onBSM(DemoSafetyMessage* bsm)
{
    dynamicTotalVehicles++;
    simtime_t processStart = simTime();

    if (bsm->getPsid() == 1) {
        dynamicEmergencyExemptions++;
    }
    else {
        bool peak = false;
        double toll = calculateTollAmount(peak);
        dynamicRevenue += toll;
        if (peak) peakHourContacts++;
    }

    transactionTimes.push_back((simTime() - processStart).dbl() * 1000.0);
}

void RSUApp::onWSM(BaseFrame1609_4* wsm) {}

void RSUApp::handleSelfMsg(cMessage* msg)
{
    DemoBaseApplLayer::handleSelfMsg(msg);
}

void RSUApp::finish()
{
    DemoBaseApplLayer::finish();

    int passengerVehicles = dynamicTotalVehicles - dynamicEmergencyExemptions;

    recordScalar("totalV2XContacts",      dynamicTotalVehicles);
    recordScalar("emergencyExempted",     dynamicEmergencyExemptions);
    recordScalar("passengerVehicles",     passengerVehicles);
    recordScalar("totalRevenue",          dynamicRevenue);
    recordScalar("peakHourContacts",      peakHourContacts);
    recordScalar("falseExemptions",       falseExemptions);

    double tollAccuracy = (passengerVehicles > 0)
        ? (double)(passengerVehicles - falseExemptions) / passengerVehicles * 100.0
        : 0.0;
    recordScalar("tollAccuracy",          tollAccuracy);

    double revenuePerVehicle = (passengerVehicles > 0)
        ? dynamicRevenue / passengerVehicles
        : 0.0;
    recordScalar("revenuePerVehicle",     revenuePerVehicle);

    double peakRevenue    = peakHourContacts * 3.75;
    double offPeakRevenue = (passengerVehicles - peakHourContacts) * 2.50;
    recordScalar("peakHourRevenue",       peakRevenue);
    recordScalar("offPeakRevenue",        offPeakRevenue);
    recordScalar("revenueReconciliation", peakRevenue + offPeakRevenue);

    double avgTxTime = transactionTimes.empty() ? 0.0
        : std::accumulate(transactionTimes.begin(), transactionTimes.end(), 0.0)
          / transactionTimes.size();
    recordScalar("avgTransactionTimeMs", avgTxTime);
}

} // <--- END NAMESPACE
