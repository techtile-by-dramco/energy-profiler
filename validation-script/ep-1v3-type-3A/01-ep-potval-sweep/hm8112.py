import serial
import csv
import time
import struct
import os

ts = round(time.time())

PORT = "COM5"
BAUDRATE = 9600

RESULT_CMD= "01A0\n"
STATUS_CMD = "02C2\n"

range_LUT = {
    0: 1,
    1: 1e3,
    2: 1e3,
    3: 1e3,
    4: 1e6,
    5: 1e7
}

def send_command(cmd):
    with serial.Serial(PORT, BAUDRATE, timeout=1) as ser:
        # Stuur commandos
        ser.write(cmd.encode("ascii"))

        ser.flush()

        response = ser.readline().decode("ascii", errors="replace").strip()

        return response

def get_resistance():

    # Retrieve measured resistance
    response = send_command(RESULT_CMD)

    # Retrieve range
    range = int(send_command(STATUS_CMD)) - 40

    # Convert to float
    try:
        measured = round(float(response)* range_LUT[range], 2)
    except ValueError:
        measured = None

    return measured

# print(get_resistance())


