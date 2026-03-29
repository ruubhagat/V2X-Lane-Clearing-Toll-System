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
    results_dir = '../examples/veins/results'
    
    v2x_data = load_config(results_dir, 'Proposed_V2X')
    siren_data = load_config(results_dir, 'Baseline_SirenOnly')

    if not v2x_data or not siren_data:
        print("Error: Could not find data files.")
        return

    v2x_tt_mean, v2x_tt_std = calc_stats(v2x_data, 'emergencyTravelTime')
    siren_tt_mean, siren_tt_std = calc_stats(siren_data, 'emergencyTravelTime')

    v2x_lc_mean, v2x_lc_std = calc_stats(v2x_data, 'laneChangesPerformed')
    siren_lc_mean, siren_lc_std = calc_stats(siren_data, 'laneChangesPerformed')

    csv_file = 'itsc_graph_data.csv'
    with open(csv_file, mode='w') as f:
        f.write("Metric,Siren-Only Mean,Siren-Only StdDev,V2X Mean,V2X StdDev\n")
        f.write("Travel Time (s),{0:.2f},{1:.2f},{2:.2f},{3:.2f}\n".format(siren_tt_mean, siren_tt_std, v2x_tt_mean, v2x_tt_std))
        f.write("Lane Changes,{0:.2f},{1:.2f},{2:.2f},{3:.2f}\n".format(siren_lc_mean, siren_lc_std, v2x_lc_mean, v2x_lc_std))

    print("Success! Data exported to: tools/" + csv_file)

if __name__ == "__main__":
    main()
