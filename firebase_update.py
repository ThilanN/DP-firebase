import firebase_admin
from firebase_admin import credentials, db
import time

# Initialize Firebase
cred = credentials.Certificate("serviceAccountKey.json")
firebase_admin.initialize_app(cred, {"databaseURL": "https://smart-parking-system-5376-default-rtdb.asia-southeast1.firebasedatabase.app"})
ref_entry = db.reference("entry")
ref_exit = db.reference("exit")

def update_firebase(gate, distance, motion, gate_open):
    if gate == "Entry":
        data = {"distance": distance, "motion": motion, "gate_open": gate_open}
        ref_entry.set(data)
        print(f"Updated Firebase Entry with: {data}")
    elif gate == "Exit":
        data = {"distance": distance, "motion": motion, "gate_open": gate_open}
        ref_exit.set(data)
        print(f"Updated Firebase Exit with: {data}")

def monitor_input():
    print("Enter data from Wokwi (format: Entry/Exit,distance,motion,gate_open) or 'quit' to exit")
    while True:
        data_input = input("Paste data here (e.g., Entry,21,0,0): ")
        if data_input.lower() == 'quit':
            break
        try:
            if data_input.startswith("DATA,"):
                data_input = data_input.replace("DATA,", "")
            gate, distance, motion, gate_open = data_input.split(",")
            distance, motion, gate_open = map(int, [distance, motion, gate_open])
            update_firebase(gate, distance, motion, gate_open)
        except ValueError as e:
            print(f"Invalid input: {e}, use format like 'Entry,21,0,0'")
        time.sleep(5)

if __name__ == "__main__":
    monitor_input()