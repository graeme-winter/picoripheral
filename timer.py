import random
import smbus
import struct
import time

from RPi import GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setup(16, GPIO.OUT)


def trigger():
    GPIO.output(16, GPIO.HIGH)
    time.sleep(0.01)
    GPIO.output(16, GPIO.LOW)


ADDRESS = 0x42

bus = smbus.SMBus(1)

# arm device
reader = struct.pack("IIII", 0, 10, 990, 10000)
driver = struct.pack("IIII", 10000, 5000, 5000, 0)

bus.write_i2c_block_data(ADDRESS, 0x10, list(reader))
bus.write_i2c_block_data(ADDRESS, 0x11, list(driver))
bus.write_i2c_block_data(ADDRESS, 0xFF, [])

# trigger
trigger()

time.sleep(12)

# arm device
reader = struct.pack("IIII", 0, 1, 99, 100000)
driver = struct.pack("IIII", 0, 500, 500, 0)

bus.write_i2c_block_data(ADDRESS, 0x10, list(reader))
bus.write_i2c_block_data(ADDRESS, 0x11, list(driver))
bus.write_i2c_block_data(ADDRESS, 0xFF, [])

# trigger
trigger()

time.sleep(12)

GPIO.cleanup()
