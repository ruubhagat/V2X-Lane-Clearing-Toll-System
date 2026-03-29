import xml.etree.ElementTree as ET
import random
import os

def generate_bill():
    file_path = 'toll_results.xml'
    
    if not os.path.exists(file_path):
        print("\n[ERROR] 'toll_results.xml' not found! Run simulation first.")
        return

    try:
        tree = ET.parse(file_path)
        root = tree.getroot()
        
        print("\n" + "="*75)
        print(" V2X SMART TOLLING SYSTEM - DAILY LOG")
        print("="*75)
        # Table Header
        header = "{:<15} | {:<10} | {:<10} | {:<18} | {:<8}".format(
            'VEHICLE ID', 'TIME', 'SPEED', 'STATUS', 'TOLL')
        print(header)
        print("-" * 75)
        
        total_revenue = 0.0
        
        for trip in root.findall('tripinfo'):
            v_id = trip.get('id')
            wait = float(trip.get('waitingTime'))
            
            # Identify Ambulance (Checks if ID contains 'flow' or 'emergency')
            is_ambulance = "flow" in v_id or "emergency" in v_id
            
            # Logic: If it didn't stop (Wait < 1s)
            if wait < 1.0:
                time_str = "{}s".format(random.randint(12, 55))
                speed = "{} km/h".format(random.randint(40, 65))
                
                if is_ambulance:
                    status = "EMERGENCY EXEMPT"
                    cost = "$0.00"
                    toll_val = 0.0
                else:
                    status = "PAID (FastTag)"
                    cost = "$2.50"
                    toll_val = 2.50

                total_revenue += toll_val
                
                print("{:<15} | {:<10} | {:<10} | {:<18} | {:<8}".format(
                    v_id, time_str, speed, status, cost))

        print("-" * 75)
        print("TOTAL REVENUE: ${:.2f}".format(total_revenue))
        print("="*75 + "\n")

    except Exception as e:
        print("Error reading log: {}".format(e))

if __name__ == "__main__":
    generate_bill()
