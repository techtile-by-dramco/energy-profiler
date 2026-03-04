import serial
import csv
import time
import struct
import os
from hm8112 import *
from ep_helper import *

ts = round(time.time())
status = "ADS-connected"

CSV_FILE = f"{ts}_{status}_results.csv"

NUM_MEASUREMENTS = 255 + 232

file_exists = os.path.isfile(CSV_FILE)

with open(CSV_FILE, mode="a", newline="") as f:

    writer = csv.writer(f)

    # Header alleen schrijven bij nieuw bestand
    if not file_exists:
        writer.writerow(["index", "epc", "measured"])
        f.flush()

    # Sweep resistance values
    for i in range(0,NUM_MEASUREMENTS,10):

        ep_change_resistance(i)
        time.sleep(2)

        measured = get_resistance()

        # EPC berekening
        # if i < 232:
        #     epc = (256 - i) * 1_000_000 / 256
        # else:
        #     epc = (256 - (i - 232)) * 94_300 / 256
        #     epc = (epc * 1e6) / (epc + 1e6)

        ep_resistance = ep_get_resistance()
        print(f"Energy Profiler Resistance: {ep_resistance}")

        writer.writerow([i, round(ep_resistance, 2), measured if measured is not None else None])
        f.flush()  # <-- cruciaal voor live groei

        print(f"{i}: {measured}")

        time.sleep(0.2)

print("Klaar → CSV live aangevuld:", CSV_FILE)