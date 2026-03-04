import serial
import csv
import time
import os
from hm8112 import *
from ep_handler import *

# Path to save the experiment data as a YAML file
current_file_path = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file_path)
parent_path = os.path.dirname(current_dir)
filename = os.path.basename(current_dir)

file_path = f"{current_dir}\{round(time.time())}_measurements.csv"

NUM_MEASUREMENTS = 255 + 232 + 232

START = 0
STOP = NUM_MEASUREMENTS

# START = 460
# STOP = 480

INTERVAL = 1

file_exists = os.path.isfile(file_path)

with open(file_path, mode="a", newline="") as f:

    writer = csv.writer(f)

    # Define header for the CSV file
    # - epc = energy profiler calculated resistance value
    # - measured = resistance value measured by the HM8112 multimeter
    if not file_exists:
        writer.writerow(["index", "epc", "measured"])
        f.flush()

    # Sweep resistance values
    for i in range(START,STOP,INTERVAL):

        ep_change_resistance(i)
        time.sleep(1)
        ep_change_resistance(i)
        time.sleep(0.5)

        #   Measure the resistance using the HM8112 multimeter
        measured = get_resistance()
        print(f"HM8112 Resistance: {measured}")

        #   Get the resistance value from the Energy Profiler
        epc = get_ep_data()["resistance"]
        print(f"Energy Profiler Resistance: {epc}")

        #   Store the results in the CSV file
        writer.writerow([i, round(epc, 2), measured if measured is not None else None])
        f.flush()  # <-- cruciaal voor live groei

        print(f"{i}: {measured}")

        # time.sleep(0.5)

print("Results saved to:", file_path)