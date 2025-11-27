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

# Create a ZeroMQ context
context = zmq.Context()

# Create a ZeroMQ PUSH socket
socket = context.socket(zmq.PUB)
socket.bind("tcp://0.0.0.0:5556")  # Bind to local address and port


ser = serial.Serial('/dev/ttyUSB1', 115200, timeout=1)  # Change 'COM1' to your serial port

def read_data():

  while(ser.in_waiting > 0):
    # Check if header is correct
    while(ser.read(2) != b'\x02\x0E'):
      None
    
    print("Serial header received successfully.")

    # Read 12 bytes (3x uint32_t)
    data = ser.read(12)

    # Unpack data into three uint32_t values (big-endian format)
    values = struct.unpack('>LLL', data)  # Assuming big-endian format

    # Print received values
    print("Received values:", values)

    return values


while 1:
  readings = read_data()
  if readings != None:

    #  Create dict
    data = dict(
      buffer_voltage_mv = readings[0],
      resistance = readings[1],
      pwr_nw = readings[2],
      )
    
    # Debug script
    print(data)
    
    #   Send JSON data over the socket
    socket.send_string(json.dumps(data))

# Close the socket and ZeroMQ context
socket.close()
context.term()
