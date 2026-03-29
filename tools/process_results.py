import glob
import sys
import os
import math
from collections import defaultdict

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    results_dir = os.path.join(script_dir, '../examples/veins/results')
    sca_files = glob.glob(os.path.join(results_dir, '*.sca'))
    
    if not sca_files:
        print("Error: No .sca files found in " + results_dir)
        sys.exit(1)
        
    metrics_data = defaultdict(list)
    total_beacons_sent = 0
    total_beacons_received = 0
    num_passenger_cars = 0
    
    for f in sca_files:
        run_lane_changes = 0
        with open(f, 'r') as file:
            for line in file:
                if line.startswith('scalar'):
                    parts = line.split()
                    if len(parts) >= 4:
                        module = parts[1]
                        metric = parts[2]
                        try:
                            val = float(parts[3])
                            
                            if metric == "laneChangesPerformed":
                                run_lane_changes += val
                            if metric == "emergencyBeaconsSent":
                                total_beacons_sent += val
                            if metric == "beaconsReceived":
                                total_beacons_received += val
                                if "node" in module or "car" in module:
                                    num_passenger_cars += 1
                            
                            # Do not drop zeros for our main metrics!
                            if val > 0 or metric in ["totalRevenue", "tollAccuracy", "falseExemptions", "emergencyExempted", "spoofingAttempts", "totalV2XContacts", "passengerVehicles"]:
                                metrics_data[metric].append(val)
                                
                        except ValueError:
                            pass
        metrics_data["SystemTotalLaneChanges"].append(run_lane_changes)

    def print_row(label, metric_key, is_currency=False, is_percent=False, force_val=None):
        if force_val is not None:
            mean_val = force_val
            std_val = 0.0
        else:
            values = metrics_data.get(metric_key, [])
            if not values:
                print("{0:30s}: 0.00 (No Events)".format(label))
                return
            mean_val = sum(values) / len(values)
            std_val = 0.0
            if len(values) > 1:
                variance = sum((x - mean_val) ** 2 for x in values) / (len(values) - 1)
                std_val = math.sqrt(variance)
            
        prefix = "$" if is_currency else ""
        suffix = "%" if is_percent else ""
        print("{0:30s}: {1}{2:8.2f}{3} +/- {4:6.2f}".format(label, prefix, mean_val, suffix, std_val))

    print("============================================================")
    print("      TABLE 1: RSU TOLL COLLECTION METRICS")
    print("============================================================")
    print_row("Total V2X Contacts", "totalV2XContacts")
    print_row("Passenger Vehicles Tolled", "passengerVehicles")
    print_row("Emergency Exemptions", "emergencyExempted")
    print_row("Total Revenue Collected", "totalRevenue", is_currency=True)
    print_row("Toll Accuracy", "tollAccuracy", is_percent=True)
    print_row("False Exemption Rate", "falseExemptions")

    print("\n============================================================")
    print("      TABLE 2: EMERGENCY VEHICLE KINEMATICS")
    print("============================================================")
    print_row("Travel Time (s)", "emergencyTravelTime")
    print_row("Distance Traveled (m)", "emergencyDistance")
    print_row("Average Speed (m/s)", "emergencyAvgSpeed")
    print_row("V2X Beacons Broadcasted", "emergencyBeaconsSent")

    print("\n============================================================")
    print("      TABLE 3: V2X NETWORK & TRAFFIC IMPACT")
    print("============================================================")
    print_row("Total Lane Changes (System)", "SystemTotalLaneChanges")
    print_row("Avg Response Delay (ms)", "avgResponseTime")
    print_row("Avg V2X Latency (ms)", "avgLatency")
    
    # Calculate System PDR safely
    pdr = 0.0
    if total_beacons_sent > 0 and num_passenger_cars > 0:
        max_possible = total_beacons_sent * num_passenger_cars
        pdr = (total_beacons_received / max_possible) * 100.0
        if pdr > 100: pdr = 100.0
        
    print_row("Packet Delivery Ratio", "packetDeliveryRatio", is_percent=True, force_val=pdr)
    print_row("Spoofing Attacks Detected", "spoofingAttempts")
    print("============================================================\n")

if __name__ == "__main__":
    main()
