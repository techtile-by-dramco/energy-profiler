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
import time

# --------------------------
# Setup Serial
# --------------------------
ser = serial.Serial('COM4', 115200, timeout=1)
print(ser)

# --------------------------
# Variables
# --------------------------
last_send_time = time.time()
latest_data = None  # Store the most recent reading

def xor_checksum(data: bytes) -> int:
    """Calculate XOR checksum over all bytes."""
    c = 0
    for b in data:
        c ^= b
    return c

def read_data():

    # clear buffer
    ser.reset_input_buffer()

    # Zoek startbyte 0x02
    while ser.read(1) != b'\x02':
        pass

    # Lees lengtebyte
    length_byte = ser.read(1)
    if len(length_byte) == 0:
        return None

    length = length_byte[0]
    print(f"Verwacht frame lengte: {length} bytes")

    # Lees payload + checksum
    frame = ser.read(length)
    if len(frame) != length:
        return None

    # frame bevat nu:
    #   [0..11] payload  (3× uint32)
    #   [12]   checksum

    payload = frame[:-1]
    recv_checksum = frame[-1]

    # Bereken checksum over: startbyte + lengthbyte + payload
    calc_checksum = xor_checksum(b'\x02' + length_byte + payload)

    if calc_checksum != recv_checksum:
        print(f"Checksum mismatch! recv={recv_checksum} calc={calc_checksum}")
        return None

    # Decodeer big-endian uint32 ×3
    # values = struct.unpack(">llll", payload) # signed 32-bit
    values = struct.unpack(">IIII", payload) # unsigned 32-bit
    return values


def get_ep_data():
    readings = read_data()
    if readings is not None:
        timestamp = round(time.time_ns() / 1e6)  # timestamp in ms
        data = {
            "timestamp": timestamp,
            "buffer_voltage_mv": readings[0],
            "resistance": readings[1],
            "pwr_pw": readings[2],
            "pot_val": readings[3]
        }
        return data
    return None

def set_ep_target_voltage(value):

    # Frame parameters
    START_BYTE = 0x02
    CMD = 0x02
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
    # with serial.Serial(PORT, BAUDRATE, timeout=1) as ser:
    time.sleep(0.1)  # korte delay voor stabiliteit
    ser.write(frame)
    ser.flush()
    # ser.close()

    print("Frame verzonden.")

def ep_change_resistance(value):

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

    time.sleep(0.1)  # korte delay voor stabiliteit
    ser.write(frame)
    ser.flush()
    # ser.close()

    print("Frame verzonden.")