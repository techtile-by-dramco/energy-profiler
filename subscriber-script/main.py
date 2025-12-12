import zmq
import json

# ---- Config ----
IP = "ttrpi5.local"
PORT = "5556"

# ---- ZMQ setup ----
context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.connect(f"tcp://{IP}:{PORT}")
socket.setsockopt_string(zmq.SUBSCRIBE, "")
socket.setsockopt(zmq.RCVTIMEO, 1000)   # 1s timeout

print("Listening for energy profiler data...\n")

try:
    while True:
        try:
            msg = socket.recv_string()
            data = json.loads(msg)

            # Verwachte structuur: { "timestamp": ..., "buffer_voltage_mv": ..., "resistance": ..., "pwr_nw": ... }
            print(
                f"{data['timestamp']} s | "
                f"{data['buffer_voltage_mv']} mV | "
                f"{data['resistance']} Ω | "
                f"{data['pwr_pw']} pW"
            )

        except zmq.error.Again:
            # timeout → stil blijven
            continue

except KeyboardInterrupt:
    print("\nStopping...")

finally:
    socket.close()
    context.term()
    print("ZMQ connection closed.")
