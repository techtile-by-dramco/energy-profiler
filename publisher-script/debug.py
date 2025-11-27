import serial

# Pas deze instellingen aan indien nodig
PORT = "/dev/ttyUSB1"      # bijvoorbeeld: "COM3", "/dev/ttyUSB0", "/dev/ttyACM0"
BAUD = 460800

def main():
    with serial.Serial(PORT, BAUD, timeout=1) as ser:
        data = ser.read(100)   # lees exact 100 bytes (of minder als timeout)
        print("Ontvangen bytes:")
        print(data)            # toont in raw vorm, bv b'\x01\x02A...'

        # Als je hex-weergave wil:
        print("Hex weergave:")
        print(" ".join(f"{b:02X}" for b in data))

if __name__ == "__main__":
    main()
