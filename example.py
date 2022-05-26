from picoripheral import Picoripheral

pico = Picoripheral()
pico.set_reader(0, 50, 50, 60000)
pico.set_driver(0, 100000, 100000, 0)
pico.arm()
pico.trigger()
pico.wait()
data = pico.read()
time = pico.time()

for t, d in zip(time, data):
    print(t, d)
