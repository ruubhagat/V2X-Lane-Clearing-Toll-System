//
// RSUApp.ned - Network Description for RSU Application
//
// Compatible with: Veins 5.1, OMNeT++ 5.5.1
//

package org.car2x.veins.modules.application.traci;

import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

//
// Road Side Unit application for emergency vehicle priority
//
// This module implements RSU functionality for detecting emergency vehicles
// and broadcasting priority clearance messages to enhance V2V communication range.
//
simple RSUApp extends DemoBaseApplLayer
{
    parameters:
        @class(veins::RSUApp);
        @display("i=block/control");
        
        // Inherits all parameters from DemoBaseApplLayer
}
