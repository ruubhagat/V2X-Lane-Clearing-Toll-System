# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('Agg')
import glob
import os
import math

# IEEE Publication Style Adjustments
plt.rcParams.update({
    'font.size': 11,
    'font.family': 'serif',
    'figure.dpi': 300,
    'savefig.dpi': 300,
    'savefig.bbox': 'tight',
    'axes.labelsize': 12,
    'axes.titlesize': 13,
    'axes.labelweight': 'bold'
})

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
    mean = sum(vals) / len(vals)
    std = 0
    if len(vals) > 1:
        std = math.sqrt(sum((x - mean)**2 for x in vals) / (len(vals) - 1))
    return mean, std

def generate_figures():
    results_dir = '../examples/veins/results'
    output_dir = '../examples/veins/figures'
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Load exactly the configurations we just ran
    v2x_data = load_config(results_dir, 'Proposed_V2X')
    siren_data = load_config(results_dir, 'Baseline_SirenOnly')

    if not v2x_data or not siren_data:
        print("Error: Could not find .sca files for V2X or SirenOnly in {0}".format(results_dir))
        return

    print("============================================================")
    print("  GENERATING ITSC PUBLICATION FIGURES")
    print("============================================================")

    # ---------------------------------------------------------
    # Figure 1: Travel Time Comparison
    # ---------------------------------------------------------
    v2x_mean, v2x_std = calc_stats(v2x_data, 'emergencyTravelTime')
    siren_mean, siren_std = calc_stats(siren_data, 'emergencyTravelTime')

    fig, ax = plt.subplots(figsize=(6, 4))
    bars = ax.bar(['Siren-Only\n(Baseline)', 'V2X\n(Proposed)'], [siren_mean, v2x_mean], 
                  yerr=[siren_std, v2x_std], capsize=8, color=['#e74c3c', '#27ae60'], edgecolor='black')
    
    for bar, mean, std in zip(bars, [siren_mean, v2x_mean], [siren_std, v2x_std]):
        ax.text(bar.get_x() + bar.get_width()/2., bar.get_height() + std + 2,
                "{0:.1f}s\n+/-{1:.1f}".format(mean, std), ha='center', va='bottom', fontweight='bold')
        
    ax.set_ylabel('Emergency Vehicle Travel Time (s)', fontweight='bold')
    ax.set_title('Emergency Response Performance', fontweight='bold')
    y_max = max(siren_mean + siren_std, v2x_mean + v2x_std)
    ax.set_ylim(0, max(y_max * 1.3, 10))
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'fig1_travel_time.pdf'))
    plt.close()
    print("Success: Saved fig1_travel_time.pdf")

    # ---------------------------------------------------------
    # Figure 2: Lane Changes Comparison
    # ---------------------------------------------------------
    v2x_lc_mean, v2x_lc_std = calc_stats(v2x_data, 'laneChangesPerformed')
    siren_lc_mean, siren_lc_std = calc_stats(siren_data, 'laneChangesPerformed')
    
    fig, ax = plt.subplots(figsize=(6, 4))
    bars = ax.bar(['Siren-Only\n(Baseline)', 'V2X\n(Proposed)'], [siren_lc_mean, v2x_lc_mean], 
                  yerr=[siren_lc_std, v2x_lc_std], capsize=8, color=['#e67e22', '#3498db'], edgecolor='black')
    
    for bar, mean, std in zip(bars, [siren_lc_mean, v2x_lc_mean], [siren_lc_std, v2x_lc_std]):
        ax.text(bar.get_x() + bar.get_width()/2., bar.get_height() + std + 2,
                "{0:.1f}\n+/-{1:.1f}".format(mean, std), ha='center', va='bottom', fontweight='bold')

    ax.set_ylabel('Total Lane Changes', fontweight='bold')
    ax.set_title('Passenger Vehicle Response Activity', fontweight='bold')
    y_max_lc = max(siren_lc_mean + siren_lc_std, v2x_lc_mean + v2x_lc_std)
    ax.set_ylim(0, max(y_max_lc * 1.3, 10)) 
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'fig2_lane_changes.pdf'))
    plt.close()
    print("Success: Saved fig2_lane_changes.pdf")
    print("============================================================\n")

if __name__ == "__main__":
    generate_figures()
