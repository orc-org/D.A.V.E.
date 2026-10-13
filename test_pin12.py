import Jetson.GPIO as GPIO
import time
import sys

def main():
    pin = 12
    print(f"Setting up physical pin {pin}...")
    
    # Use physical pin numbering
    GPIO.setmode(GPIO.BOARD)
    
    try:
        # Set pin as output
        GPIO.setup(pin, GPIO.OUT)
        
        print("Starting 1-second toggle loop. Press Ctrl+C to stop.")
        while True:
            print("Turning ON pin 12...")
            GPIO.output(pin, GPIO.HIGH)
            time.sleep(1)
            
            print("Turning OFF pin 12...")
            GPIO.output(pin, GPIO.LOW)
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\nLoop stopped by user.")
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
    finally:
        # Always clean up GPIO state
        print("Cleaning up...")
        GPIO.cleanup()

if __name__ == "__main__":
    main()
