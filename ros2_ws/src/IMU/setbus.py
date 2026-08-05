import time
import board
import busio
from adafruit_bno08x import (
    BNO_REPORT_ACCELEROMETER,
    BNO_REPORT_GYROSCOPE,
)
from adafruit_bno08x.i2c import BNO08X_I2C

# explicitly open I2C bus 7 (pins 3 & 5 on the 40-pin header)
# on the jetson because of camera overlays, physical pins 3/5 map to /dev/i2c-7
i2c = busio.I2C(board.SCL, board.SDA, frequency=400000)

# if board.SCL defaults to Bus 1 and throws an error, force Bus 7 directly:
# import adafruit_extended_bus
# i2c = adafruit_extended_bus.ExtendedI2C(7)

bno = BNO08X_I2C(i2c, address=0x4A)

# Enable sensors
bno.enable_feature(BNO_REPORT_ACCELEROMETER)
bno.enable_feature(BNO_REPORT_GYROSCOPE)

print("BNO085 initialized successfully on Bus 7!")