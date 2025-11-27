#  ____  ____      _    __  __  ____ ___
# |  _ \|  _ \    / \  |  \/  |/ ___/ _ \
# | | | | |_) |  / _ \ | |\/| | |  | | | |
# | |_| |  _ <  / ___ \| |  | | |__| |_| |
# |____/|_| \_\/_/   \_\_|  |_|\____\___/
#                           research group
#                             dramco.be/
#
#  KU Leuven - Technology Campus Gent,
#  Gebroeders De Smetstraat 1,
#  B-9000 Gent, Belgium
#
#         File: main.py
#      Created: 2024-04-30
#       Author: Jarne Van Mulders
#      Version: 1.0
#
#  Description: Publish energy profiler data via ZMQ
#      Energy profiler motherboard sends its data via BLE
#
#  Commissiond by company C (optionally)
#
#  License L (optionally)
#
import serial
import struct
import zmq
import json
import time

# --------------------------
# Setup ZeroMQ
# --------------------------
context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("tcp://0.0.0.0:5556")

# --------------------------
# Setup Serial
# --------------------------
ser = serial.Serial('/dev/ttyUSB1', 460800, timeout=1)
print(ser)

# --------------------------
# Variables
# --------------------------
last_send_time = time.time()
latest_data = None  # Store the most recent reading

# --------------------------
# Function to read a single measurement
# --------------------------
def read_data():
    global ser
    # Wait for header b'\x02\x0E'
    while True:
        header = ser.read(2)
        if header == b'\x02\x0E':
            break

    # Read 12 bytes (3 x uint32)
    raw_data = ser.read(12)
    if len(raw_data) != 12:
        return None

    values = struct.unpack('>LLL', raw_data)  # big-endian uint32
    return values

# --------------------------
# Main loop
# --------------------------
while True:
    # Read as fast as possible
    readings = read_data()
    if readings is not None:
        timestamp = round(time.time_ns() / 1e6)  # timestamp in ms
        latest_data = {
            "timestamp": timestamp,
            "buffer_voltage_mv": readings[0],
            "resistance": readings[1],
            "pwr_nw": readings[2]
        }

    # Only transmit every 1 second
    if latest_data and (time.time() - last_send_time) >= 1.0:
        socket.send_string(json.dumps(latest_data))
        print(f"{latest_data['timestamp']} - {latest_data['buffer_voltage_mv']} - "
              f"{latest_data['resistance']} - {latest_data['pwr_nw']}")
        last_send_time = time.time()

