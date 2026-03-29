package org.car2x.veins.modules.application.traci;

import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

simple TraCIDemoRSU11p extends DemoBaseApplLayer
{
    @class(veins::RSUApp);
    @display("i=block/tower");
}
