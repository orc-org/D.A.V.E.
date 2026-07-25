import serial

PORT = '/dev/ttyUSB0'
BAUD = 38400

try:
    print(f"Opening {PORT} at {BAUD} baud...")
    # open the serial port with a 1-second timeout
    ser = serial.Serial(PORT, BAUD, timeout=1)
    
    print("Listening for NMEA data... (Press Ctrl+C to stop)\n")
    print("-" * 50)
    
    while True:
        if ser.in_waiting > 0:
            # read a line, decode it, and strip trailing newlines
            line = ser.readline().decode('ascii', errors='replace').strip()
            
            # highlight the sentences containing location coordinates
            if line.startswith('$GNGGA') or line.startswith('$GPGGA'):
                print(f"FIX DATA: {line}")
            elif line.startswith('$GNRMC') or line.startswith('$GPRMC'):
                print(f"NAV DATA: {line}")
            elif line:
                # print other satellite info normally
                print(f"   {line}")
                
except serial.SerialException as e:
    print(f"\nSerial Error: {e}")
    print("If you got a 'Permission denied' error, run the script with 'sudo',")
    print("or run 'sudo usermod -a -G dialout $USER' and reboot.")
except KeyboardInterrupt:
    print("\nTest stopped by user.")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("Serial port closed.")
