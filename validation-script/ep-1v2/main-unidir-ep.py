import serial
import csv
import time
import struct
import os
from hm8112 import *

ts = round(time.time())
status = "ADS-connected"

CSV_FILE = f"{ts}_{status}_results.csv"

NUM_MEASUREMENTS = 255 + 232

def change_resistance(value):
    # Seriële poort instellingen
    PORT = "COM4"
    BAUDRATE = 115200

    # Frame parameters
    START_BYTE = 0x02
    CMD = 0x01
    DATA_LEN = 0x04
    END_BYTE = 0xFF

    # Bouw frame: big-endian 4 bytes
    frame = bytearray()
    frame.append(START_BYTE)
    frame.append(CMD)
    frame.append(DATA_LEN)
    frame += struct.pack(">I", value)  # 4 bytes, big-endian
    frame.append(END_BYTE)

    print("Te verzenden frame:", frame.hex(" "))

    # Open UART en stuur frame
    with serial.Serial(PORT, BAUDRATE, timeout=1) as ser:
        time.sleep(0.1)  # korte delay voor stabiliteit
        ser.write(frame)
        ser.flush()

    print("Frame verzonden.")

file_exists = os.path.isfile(CSV_FILE)


with open(CSV_FILE, mode="a", newline="") as f:

    writer = csv.writer(f)

    # Header alleen schrijven bij nieuw bestand
    if not file_exists:
        writer.writerow(["index", "epc", "measured"])
        f.flush()

    # Sweep resistance values
    for i in range(NUM_MEASUREMENTS):

        change_resistance(i)
        time.sleep(2)

        measured = get_resistance()

        # EPC berekening
        if i < 232:
            epc = (256 - i) * 1_000_000 / 256
        else:
            epc = (256 - (i - 232)) * 94_300 / 256
            epc = (epc * 1e6) / (epc + 1e6)

        writer.writerow([i, round(epc, 2), measured if measured is not None else None])
        f.flush()  # <-- cruciaal voor live groei

        print(f"{i}: {measured}")

        time.sleep(0.2)

print("Klaar → CSV live aangevuld:", CSV_FILE)