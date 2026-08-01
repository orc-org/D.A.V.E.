#!/usr/bin/env python3
"""
Hardware Test Script for DM556Y Stepper Driver on Jetson Orin Nano
Wired Pins:
  PUL+ -> Jetson Pin 15 (GPIO12 / PWM)
  DIR+ -> Jetson Pin 7  (GPIO9)
  PUL-, DIR- -> Jetson GND (Pin 6/9/14)
"""

import time
import sys

try:
    import Jetson.GPIO as GPIO
except ImportError:
    print("Error: Jetson.GPIO library is not installed.")
    sys.exit(1)

PUL_PIN = 15  # Pin 15 (Hardware PWM)
DIR_PIN = 18  # Pin 12 (Direction - Verified Free)

def setup():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(PUL_PIN, GPIO.OUT, initial=GPIO.LOW)
    GPIO.setup(DIR_PIN, GPIO.OUT, initial=GPIO.LOW)

def test_pulse_frequency(target_freq_hz=20000, duration_sec=3.0, forward=True):
    """
    Ramps frequency up to target_freq_hz using Jetson Hardware PWM
    so the stepper motor accelerates smoothly without stalling.
    """
    print(f"\n-> Setting Direction: {'FORWARD (HIGH)' if forward else 'REVERSE (LOW)'}")
    GPIO.setup(DIR_PIN, GPIO.OUT)
    GPIO.output(DIR_PIN, GPIO.HIGH if forward else GPIO.LOW)
    time.sleep(0.01)

    print(f"-> Starting PWM Frequency Ramping up to {target_freq_hz/1000:.1f} kHz...")
    
    # Ensure pin is set up as OUTPUT before creating PWM
    GPIO.setup(PUL_PIN, GPIO.OUT, initial=GPIO.LOW)
    pwm = GPIO.PWM(PUL_PIN, 1000)
    pwm.start(50)  # 50% duty cycle square wave
    
    # Smooth acceleration ramp from 1kHz to target_freq_hz over 0.5s
    steps_ramp = 50
    start_freq = 1000
    for i in range(steps_ramp + 1):
        current_freq = int(start_freq + (target_freq_hz - start_freq) * (i / steps_ramp))
        pwm.ChangeFrequency(current_freq)
        time.sleep(0.01)

    print(f"-> Holding at top speed ({target_freq_hz/1000:.1f} kHz) for {duration_sec}s...")
    time.sleep(duration_sec)

    # Deceleration ramp down to 1kHz
    print("-> Decelerating...")
    for i in range(steps_ramp, -1, -1):
        current_freq = int(start_freq + (target_freq_hz - start_freq) * (i / steps_ramp))
        pwm.ChangeFrequency(current_freq)
        time.sleep(0.01)

    pwm.stop()
    print("-> Stopped.")

def main():
    print("=" * 65)
    print(" DM556Y High-Speed PWM Stepper Motor Test (Jetson Orin Nano)")
    print("=" * 65)
    print(f" PUL+ -> Jetson Pin {PUL_PIN} (Hardware PWM)")
    print(f" DIR+ -> Jetson Pin {DIR_PIN} (Direction)")
    print(f" PUL-, DIR- -> Jetson GND")
    print("-" * 65)

    target_freq = 200000  # Default 20 kHz (12.5 RPS / 750 RPM at 1600 microstep)
    if len(sys.argv) > 1:
        try:
            target_freq = int(sys.argv[1])
        except ValueError:
            pass

    print(f"Target Max Frequency: {target_freq} Hz ({target_freq/1000:.1f} kHz)")

    try:
        setup()

        # Run Forward Test
        test_pulse_frequency(target_freq_hz=target_freq, duration_sec=3.0, forward=True)
        time.sleep(1.0)

        # Run Reverse Test
        test_pulse_frequency(target_freq_hz=target_freq, duration_sec=3.0, forward=False)

        print("\n✅ Spin Test Complete!")

    except KeyboardInterrupt:
        print("\nTest interrupted by user.")
    except Exception as e:
        print(f"\n❌ Error during test: {e}")
    finally:
        try:
            GPIO.cleanup()
        except Exception:
            pass

if __name__ == '__main__':
    main()
