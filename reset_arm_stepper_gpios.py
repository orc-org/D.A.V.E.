#!/usr/bin/env python3
"""
Emergency GPIO Reset Script for Arm & Gripper DM556Y Stepper Drivers
---------------------------------------------------------------------
Forces all Jetson Orin Nano stepper control pins to solid 0.0V LOW (GND)
and cleans up GPIO state.
"""

import sys
import time

try:
    import Jetson.GPIO as GPIO
    HAS_JETSON_GPIO = True
except ImportError:
    HAS_JETSON_GPIO = False


# Jetson BOARD Pin Assignments (Matches ArmStepperDriverNode parameters)
STEPPER_PINS = {
    'ARM_STEP': 33,
    'ARM_DIR': 18,
    'ARM_ENABLE': 22,
    'GRIPPER_STEP': 32,
    'GRIPPER_DIR': 13,
    'GRIPPER_ENABLE': 16,
}


def reset_stepper_gpios():
    if not HAS_JETSON_GPIO:
        print("[ERROR] Jetson.GPIO library is not installed on this environment/system.")
        print("[INFO] Expected pins to reset: " + ", ".join([f"{k}=Pin {v}" for k, v in STEPPER_PINS.items()]))
        sys.exit(1)

    print("[INFO] Initializing Jetson Orin Nano GPIO in BOARD mode...")
    try:
        GPIO.setwarnings(False)
        GPIO.setmode(GPIO.BOARD)

        print("[INFO] Driving all Arm & Gripper Stepper pins to 0.0V LOW...")
        for name, pin in STEPPER_PINS.items():
            GPIO.setup(pin, GPIO.OUT, initial=GPIO.LOW)
            GPIO.output(pin, GPIO.LOW)
            print(f"  -> Pin {pin:2d} ({name:14s}): SET TO LOW (0.0V)")

        # Short delay to allow electrical voltage levels on signal lines to settle
        time.sleep(0.1)

        GPIO.cleanup()
        print("[SUCCESS] All stepper GPIO pins successfully forced to LOW (0.0V) and cleaned up.")
    except Exception as e:
        print(f"[ERROR] An exception occurred while resetting GPIO pins: {e}")
        try:
            GPIO.cleanup()
        except Exception:
            pass
        sys.exit(1)


if __name__ == '__main__':
    reset_stepper_gpios()
