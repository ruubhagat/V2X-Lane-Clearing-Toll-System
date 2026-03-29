package org.car2x.veins.modules.application.traci;

import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

simple MyVeinsApp extends DemoBaseApplLayer
{
    parameters:
        @class(veins::MyVeinsApp);

        int    operationMode      = default(0);
        bool   isMalicious        = default(false);
        double attackProbability  = default(0.0);
        double baseToll           = default(2.50);
        double hardwareLatency    = default(0.005);
        double driverReactionJitter = default(0.15);

        @display("i=block/app2");
}
