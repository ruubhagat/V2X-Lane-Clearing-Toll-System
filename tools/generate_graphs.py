import matplotlib.pyplot as plt
import os

def generate_pdr_chart():
    # The exact data from your two OMNeT++ simulation runs
    scenarios = ['Clean Network\n(Proposed V2X)', 'Network Under Attack\n(Spoofing/Flooding)']
    pdr_values = [86.81, 50.79]
    
    plt.figure(figsize=(7, 5))
    bars = plt.bar(scenarios, pdr_values, color=['#2ca02c', '#d62728'], width=0.5)
    
    plt.ylabel('Packet Delivery Ratio (PDR %)', fontweight='bold', fontsize=12)
    plt.title('Impact of Malicious Spoofing on V2X Reliability', fontweight='bold', fontsize=14)
    plt.ylim(0, 100)
    
    # Simple workaround for older matplotlib versions
    ax = plt.gca()
    ax.yaxis.grid(True, linestyle='--', alpha=0.7)
    
    # Add the exact numbers on top of the bars using universal formatting
    for bar in bars:
        yval = bar.get_height()
        plt.text(bar.get_x() + bar.get_width() / 2.0, yval + 2.0, "{0}%".format(yval), ha='center', va='bottom', fontweight='bold')
    
    # Create directory safely for older Python versions
    if not os.path.exists('figures'):
        os.makedirs('figures')
        
    plt.savefig('figures/pdr_impact.pdf', dpi=300, bbox_inches='tight')
    print("Success: IEEE-formatted chart saved to tools/figures/pdr_impact.pdf")

if __name__ == "__main__":
    generate_pdr_chart()
