import random
import smbus
import struct
import time

ADDRESS = 0x42

bus = smbus.SMBus(1)

reader = struct.pack("IIII", 0, 10, 990, 10000)
driver = struct.pack("IIII", 10000, 5000, 5000, 0)

bus.write_i2c_block_data(ADDRESS, 0x10, list(reader))
bus.write_i2c_block_data(ADDRESS, 0x11, list(driver))
bus.write_i2c_block_data(ADDRESS, 0xFF, [])
