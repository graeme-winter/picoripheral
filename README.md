Picoripheral - The £4 toy oscilloscope
======================================

The Raspberry Pi pico is the first microcontroller from the Raspberry Pi foundation - and it is often presented as a way of running micropython to drive motors or blink LEDs (or drive sensors etc...) but in fact it is a moderately powerful device in its own right. The two ARM Cortex-M0+ cores attract the attention but the real power comes from multiple PIO devices, DMA, a substantial amount of RAM, ADC etc. - sufficient that you can use this as a peripheral device to attach to a Raspberry Pi computer for (modest) real-time data capture. Hence, the picoripheral.

The picoripheral answers the question of reading ADC data at a reliable clock (e.g. 100µs) with perhaps synchronised driving of a circuit, saving the data to internal arrays. The tooling includes:

 - a driver circuit - allownig square wave with a set delay, high, low sequence
 - a trigger circuit for the readout
 - i2c control for set-up / arming
 - 3.3v trigger over GPIO
 - data egress via spi

The simplest application is to monitor the voltage across an RC circuit, driven by the driver clock then reading the voltage across the capacitor.

![Wiring diagram](./wiring.png)

The Python code running on the Raspberry Pi then looks like:

```python
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

points = 60000
high = 50
low = 50

# arm device
reader = struct.pack("IIII", 0, high, low, points)
driver = struct.pack("IIII", 100000, 200000, 200000, 0)

bus.write_i2c_block_data(ADDRESS, 0x10, list(reader))
bus.write_i2c_block_data(ADDRESS, 0x11, list(driver))
bus.write_i2c_block_data(ADDRESS, 0xFF, [])

# trigger
trigger()

time.sleep(1e-6 * points * (high + low) + 0.1)
GPIO.cleanup()

result = fetch(points)

for j, r in enumerate(result):
    print(j * (high + low) + high, r)
```

TODO make a proper Python API package. This outputs the following when connected to an RC consisting of 2 x 1k resistors, 100µF capacitor:

![ADC output](./adc_rc.png)
