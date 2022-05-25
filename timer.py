import random
import smbus
import struct
import time

from RPi import GPIO
import spidev

GPIO.setmode(GPIO.BCM)
GPIO.setup(16, GPIO.OUT)


def fetch(nn):
    spi = spidev.SpiDev()
    spi.open(0, 0)
    spi.mode = 3
    spi.bits_per_word = 8
    spi.max_speed_hz = 10000000

    zero = [0 for j in range(2 * nn)]
    
    data = bytearray(spi.xfer3(zero))

    # unpack uint16_t

    return struct.unpack(f"{nn}H", data)


def trigger():
    GPIO.output(16, GPIO.HIGH)
    time.sleep(0.01)
    GPIO.output(16, GPIO.LOW)


ADDRESS = 0x42

bus = smbus.SMBus(1)

points = 10000
high = 50
low = 50

# arm device
reader = struct.pack("IIII", 0, high, low, points)
driver = struct.pack("IIII", 50000, 100000, 100000, 0)

bus.write_i2c_block_data(ADDRESS, 0x10, list(reader))
bus.write_i2c_block_data(ADDRESS, 0x11, list(driver))
bus.write_i2c_block_data(ADDRESS, 0xFF, [])

# trigger
trigger()

time.sleep(2)
GPIO.cleanup()

result = fetch(points)

for j, r in enumerate(result):
    print(j * (high + low) + high, r)
