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

# Seriële poort instellingen
PORT = "COM4"
BAUDRATE = 115200

# --------------------------
# Function to read a single measurement
# --------------------------
def xor_checksum(data: bytes) -> int:
    """Calculate XOR checksum over all bytes."""
    c = 0
    for b in data:
        c ^= b
    return c

def read_data(ser):
    # Zoek startbyte 0x02
    while ser.read(1) != b'\x02':
        pass

    # Lees lengtebyte
    length_byte = ser.read(1)
    if len(length_byte) == 0:
        return None

    length = length_byte[0]

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
    values = struct.unpack(">lll", payload)
    return values

def ep_get_resistance():
    with serial.Serial(PORT, BAUDRATE, timeout=1) as ser:
        readings = read_data(ser)
        ser.close()
        if readings is not None:
            return readings[1]
        else:
            return 0 

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

    # Open UART en stuur frame
    with serial.Serial(PORT, BAUDRATE, timeout=1) as ser:
        time.sleep(0.1)  # korte delay voor stabiliteit
        ser.write(frame)
        ser.flush()
        ser.close()

    print("Frame verzonden.")



# --------------------------
# Main loop
# --------------------------
# while True:
#     # Read as fast as possible
#     readings = read_data()
#     if readings is not None:
#         timestamp = round(time.time_ns() / 1e6)  # timestamp in ms
#         latest_data = {
#             "timestamp": timestamp,
#             "buffer_voltage_mv": readings[0],
#             "resistance": readings[1],
#             "pwr_pw": readings[2]
#         }

#     # # Only transmit every 1 second
#     # if latest_data and (time.time() - last_send_time) >= 1.0:
#     #     socket.send_string(json.dumps(latest_data))
#     #     print(f"{latest_data['timestamp']} - {latest_data['buffer_voltage_mv']} - "
#     #           f"{latest_data['resistance']} - {latest_data['pwr_pw']}")
#     #     last_send_time = time.time()

#     # Only transmit every 100 ms
#     if latest_data and (time.time() - last_send_time) >= 0.1:
#         # socket.send_string(json.dumps(latest_data))
#         print(f"{latest_data['timestamp']} - {latest_data['buffer_voltage_mv']} - "
#               f"{latest_data['resistance']} - {latest_data['pwr_pw']}")
#         last_send_time = time.time()
