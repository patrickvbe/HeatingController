import rfreceiver
from machine import Pin

def callback(rcv):
    print(rcv.code_protocol, rcv.code, rcv.code_timestamp)
    if rcv.code_protocol == rfreceiver.WEATHERSTATION:
        print(rfreceiver.getTempFromWeather(rcv.code) / 10, "C")

rcv = rfreceiver.receiver(Pin(5), callback)
rcv.start()

snd = rfreceiver.sender(Pin(4))

def sendit():
    snd.send(rfreceiver.ELRO, 9717073)
