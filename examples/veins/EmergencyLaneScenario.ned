//
// EmergencyLaneScenario.ned - Network Topology Definition
//
// RESEARCH-READY IMPLEMENTATION
// Compatible with: OMNeT++ 5.5.1, Veins 5.1
//


import org.car2x.veins.nodes.RSU;
import org.car2x.veins.nodes.Scenario;

//
// Emergency Lane Scenario Network
//
// This network topology implements a highway scenario with:
// - Dynamic vehicle nodes (managed by TraCIScenarioManager)
// - Fixed Road Side Units (RSUs) for infrastructure support
// - Emergency vehicle detection and response capabilities
//
// Network Components:
// - TraCIScenarioManager: Interfaces with SUMO traffic simulation
// - RSUs: Road side units for V2I communication
// - Vehicle nodes: Dynamically created based on SUMO simulation
//
network EmergencyLaneScenario extends Scenario
{
    parameters:
        // Number of Road Side Units
        int numRsu = default(2);
        
        @display("bgb=1600,100");
        
    submodules:
        // Road Side Units
        rsu[numRsu]: RSU {
            @display("p=100,50;i=block/tower,gold");
        }
}
