import xml.etree.ElementTree as ET

def inject():
    filename = "realworld.rou.xml"
    try:
        tree = ET.parse(filename)
        root = tree.getroot()

        # 1. Define the Ambulance Type (Fast, Red, Emergency Shape)
        amb_type = ET.Element("vType")
        amb_type.set("id", "ambulance")
        amb_type.set("vClass", "emergency")
        amb_type.set("color", "255,0,0")
        amb_type.set("guiShape", "emergency")
        amb_type.set("maxSpeed", "60")  # Faster than cars
        amb_type.set("accel", "3.0")
        
        # Insert it at the top (index 0)
        root.insert(0, amb_type)

        # 2. Find a specific vehicle to convert (e.g., the 5th one)
        vehicles = root.findall("vehicle")
        if len(vehicles) > 5:
            target_car = vehicles[5]  # Index 5 is roughly t=10s to t=20s
            target_car.set("type", "ambulance")
            target_car.set("id", "toll_ambulance") # Rename it so we spot it in logs
            target_car.set("color", "255,0,0")     # Make it RED
            target_car.set("guiShape", "emergency") # Blue lights
            
            print(f"SUCCESS: Converted vehicle '{target_car.get('id')}' into an AMBULANCE.")
        else:
            print("WARNING: Not enough vehicles generated. Run randomTrips with -e 100.")

        # 3. Save the file back
        tree.write(filename)
        print("Traffic file updated successfully!")

    except Exception as e:
        print(f"Error updating file: {e}")

if __name__ == "__main__":
    inject()
