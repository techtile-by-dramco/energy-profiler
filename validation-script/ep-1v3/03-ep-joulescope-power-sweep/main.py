import csv
import os
import joulescope
import numpy as np
from ep_handler import *

# Path to save the experiment data as a YAML file
current_file_path = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file_path)
parent_path = os.path.dirname(current_dir)
filename = os.path.basename(current_dir)


file_path = f"{current_dir}\{round(time.time())}_measurements.csv"

# NUM_MEASUREMENTS = 651
START = 0
STOP = 700
STEPS = 5

def get_data(i):

    ep_change_resistance(i)

    time.sleep(3)

    ep_data = get_ep_data()

    data = js.read(contiguous_duration=0.1)
    current, voltage = np.mean(data, axis=0, dtype=np.float64)
    # print(f'{current} A, {voltage} V')

    js_data = {
        "js_current_pA": round(current * 1e12),
        "js_voltage_mv": round(voltage * 1e3),
        "js_power_pw": round(current * voltage * 1e12)
    }

    data = {**js_data, **ep_data}

    return data
    
    

with joulescope.scan_require_one(config='auto') as js, open(file_path, mode='a', newline='') as f:

    writer = None

    for i in range(START, STOP, STEPS):
        data = get_data(i)

        # Create writer only once, using real data keys
        if writer is None:
            file_is_empty = f.tell() == 0
            writer = csv.DictWriter(f, fieldnames=data.keys())

            if file_is_empty:
                writer.writeheader()

        writer.writerow(data)

        # Good for long measurements or crashes
        f.flush()