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

# Create a ZeroMQ context
context = zmq.Context()

# Create a ZeroMQ PUSH socket
socket = context.socket(zmq.PUB)
socket.bind("tcp://0.0.0.0:5556")  # Bind to local address and port

raw_data = None


ser = serial.Serial('/dev/ttyUSB1', 460800, timeout=1)  # Change 'COM1' to your serial port

print(ser)

def read_data():

  while(ser.in_waiting > 0):
    # Check if header is correct

    # val = ser.read(2)
    while(ser.read(2) != b'\x02\x0E'):
      # val = ser.read(2)
      # print(val)
      None

    # while(ser.read(1) != b'\x02'):
    # #   val = ser.read(2)
    # #   print(val)
    #   None
    
    # print("Serial header received successfully.")

    # Read 12 bytes (3x uint32_t)
    raw_data = ser.read(12)

    # Unpack data into three uint32_t values (big-endian format)
    values = struct.unpack('>LLL', raw_data)  # Assuming big-endian format
    # values = struct.unpack('<LLL', raw_data)  # Change to little-endian format

    # Print received values
    print("Received values:", values)

    return values

counter = 0
prev_time = 0

new_data = [0,0,0,0]

while 1:
  readings = read_data()
  if readings != None:

    new_data = [round(time.time_ns()/1e6)] + list(readings)
    # print(new_data)

    #  Create dict
    data = dict(
      timestamp = new_data[0],
      buffer_voltage_mv = new_data[1],
      resistance = new_data[2],
      pwr_nw = new_data[3],
      )
    
    # Debug script
    # print(data)
    
    #   Send JSON data over the socket
    socket.send_string(json.dumps(data))

    counter = counter + 1

    # if (time.time() - prev_time) > 1:
    #   prev_time = time.time()
      
    #   print(f"{counter} messages send per second")

    print(f"{data['timestamp']} - {data['buffer_voltage_mv']} - {data['resistance']} - {data['pwr_nw']}")

    #   counter = 0
    

# Close the socket and ZeroMQ context
socket.close()
context.term()
