# -*- coding: utf-8 -*-
import glob
import os
import math

def parse_sca(filepath):
    data = {}
    with open(filepath, 'r') as f:
        for line in f:
            if line.startswith('scalar'):
                parts = line.split()
                if len(parts) >= 4:
                    metric = parts[2].split(':')[-1]
                    try:
                        data[metric] = float(parts[3])
                    except ValueError:
                        pass
    return data

def load_config(results_dir, config_pattern):
    pattern = os.path.join(results_dir, "*" + config_pattern + "*.sca")
    files = glob.glob(pattern)
    if not files:
        return []
    return [parse_sca(f) for f in files]

def calc_stats(data_list, metric):
    vals = [d.get(metric) for d in data_list if metric in d and d.get(metric) is not None]
    if not vals:
        return 0, 0
    mean = sum(vals) / float(len(vals))
    std = 0
    if len(vals) > 1:
        std = math.sqrt(sum((x - mean)**2 for x in vals) / float(len(vals) - 1))
    return mean, std

def main():
    print("============================================================")
    print("  EXTRACTING DATA FOR ITSC FIGURES 1-5")
    print("============================================================")
    
    results_dir = '../examples/veins/results'
    output_file = 'itsc_all_figures_data.csv'

    v2x_data = load_config(results_dir, 'Proposed_V2X')
    siren_data = load_config(results_dir, 'Baseline_SirenOnly')

    if not v2x_data:
        print("Error: No V2X simulation data found.")
        return

    with open(output_file, 'w') as f:
        # ---------------------------------------------------------
        # Figures 1 & 2: Comparisons
        # ---------------------------------------------------------
        f.write("--- FIGURE 1 & 2: BASELINE COMPARISONS ---\n")
        f.write("Metric,Siren-Only Mean,Siren-Only StdDev,V2X Mean,V2X StdDev\n")
        
        siren_tt_m, siren_tt_s = calc_stats(siren_data, 'emergencyTravelTime')
        v2x_tt_m, v2x_tt_s = calc_stats(v2x_data, 'emergencyTravelTime')
        f.write("Travel Time (s),{0:.2f},{1:.2f},{2:.2f},{3:.2f}\n".format(siren_tt_m, siren_tt_s, v2x_tt_m, v2x_tt_s))

        siren_lc_m, siren_lc_s = calc_stats(siren_data, 'laneChangesPerformed')
        v2x_lc_m, v2x_lc_s = calc_stats(v2x_data, 'laneChangesPerformed')
        f.write("Lane Changes,{0:.2f},{1:.2f},{2:.2f},{3:.2f}\n\n".format(siren_lc_m, siren_lc_s, v2x_lc_m, v2x_lc_s))

        # ---------------------------------------------------------
        # Figures 3, 4, & 5: V2X Performance Metrics
        # ---------------------------------------------------------
        f.write("--- FIGURE 3, 4, & 5: V2X PERFORMANCE ---\n")
        f.write("Metric,V2X Mean,V2X StdDev\n")
        
        # Figure 3: Toll Collection
        toll_m, toll_s = calc_stats(v2x_data, 'passengerVehicles')
        f.write("Toll Transactions,{0:.2f},{1:.2f}\n".format(toll_m, toll_s))
        exempt_m, exempt_s = calc_stats(v2x_data, 'emergencyExempted')
        f.write("Emergency Exemptions,{0:.2f},{1:.2f}\n".format(exempt_m, exempt_s))
        rev_m, rev_s = calc_stats(v2x_data, 'totalRevenue')
        f.write("Revenue ($),{0:.2f},{1:.2f}\n".format(rev_m, rev_s))

        # Figure 4: Network
        pdr_m, pdr_s = calc_stats(v2x_data, 'packetDeliveryRatio')
        f.write("Packet Delivery Ratio (%),{0:.2f},{1:.2f}\n".format(pdr_m, pdr_s))

        # Figure 5: Speed/Kinematics
        speed_m, speed_s = calc_stats(v2x_data, 'emergencyAvgSpeed')
        f.write("Avg Speed (m/s),{0:.2f},{1:.2f}\n".format(speed_m, speed_s))
        dist_m, dist_s = calc_stats(v2x_data, 'emergencyDistance')
        f.write("Distance Traveled (m),{0:.2f},{1:.2f}\n".format(dist_m, dist_s))

    print("Success! Complete figure data exported to: tools/" + output_file)

if __name__ == "__main__":
    main()
