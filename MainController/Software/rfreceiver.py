import time
import micropython
from collections import namedtuple
from machine import Pin
import array

micropython.alloc_emergency_exception_buf(150)

Protocol = namedtuple('Protocol',
                      ('id', 'signallength',
                       'start_high', 'start_low',
                       'zero_high', 'zero_low',
                       'one_high', 'one_low',
                       'end_high', 'end_low'))

SMARTWARES = 1
WEATHERSTATION = 2
CONRADRSL = 3
ELRO = 4

PROTOCOLS = (
    #Protocol(SMARTWARES,    132, 300, 2500, 300,  300,  300, 1200, 300, 10000),     # Smartwares (Action)
    Protocol(SMARTWARES,    130,   0,    0, 300,  300,  300, 1200, 300, 10000),     # Smartwares (Action)
    Protocol(WEATHERSTATION, 76,   0,    0, 525,  925,  525, 1850, 500,  3850),     # Weatherstation
    Protocol(CONRADRSL,      66,   0,    0, 600, 1200, 1200,  600, 600,  7000),     # Conrad RSL
    Protocol(ELRO,           50,   0,    0, 325,  975,  975,  325, 325, 10000)      # Elro
)


class recorder:
    MAXRECORD = 500
    data = array.array('L', (0,) * MAXRECORD)
    pin = Pin(5)
    led = Pin(2)
    led.init(Pin.OUT)
    last_timestamp = time.ticks_us()
    counter = 0

    def decode(self):
        for protocol in PROTOCOLS:
            if self.decodeprotocol(protocol):
                return True
        return False

    def decodeprotocol(self, protocol):
        if self.counter < protocol.signallength:
            #print("check1 failed")
            return False
        start = self.counter - protocol.signallength
        if protocol.start_low != 0:
            if self.data[start + 1] < (protocol.start_low * 2) // 3:
                #print("check2 failed")
                return False
            start += 2
        if protocol.end_low < (self.data[-1] * 2) // 3:
            #print("check3 failed")
            return False
        # pre-calculate ranges for speed...
        zerohl = (protocol.zero_high) // 2
        zerohh = zerohl * 3
        zeroll = (protocol.zero_low) // 2
        zerolh = zeroll * 3
        onehl = (protocol.one_high) // 2
        onehh = onehl * 3
        onell = (protocol.one_low) // 2
        onelh = onell * 3
        #print(zerohl, zerohh, zeroll, zerolh, onehl, onehh, onell, onelh)
        code = 0
        for pos in range(start, self.counter - 2, 2): # -2 removes the end-sync they all have.
            high = self.data[pos]
            low = self.data[pos+1]
            if zerohl < high and  high < zerohh and zeroll < low and low < zerolh:
                code <<= 1
            elif onehl < high and  high < onehh and onell < low and low < onelh:
                code <<= 1
                code |= 1
            else:
                if protocol.signallength == 130:
                    print("bit failed", pos, high, low)
                return False
        if code == 0:
            return False
        print("Decoded: {} {:64b}".format(protocol.id, code))
        return True

    def rxinterrupt(self, pin):
        timestamp = time.ticks_us()
        duration = time.ticks_diff(timestamp, self.last_timestamp)
        self.last_timestamp = timestamp

        # Pulses shorter than 250us are usually noise. But my smartwares remote drifts to almost 225us...
        if duration > 250 or (duration > 225 and self.counter > 0):
            self.data[self.counter] = duration
            self.counter += 1
            if duration > 3500 or self.counter == self.MAXRECORD:
                self.led.value(not self.led.value())
                self.decode()
                self.counter = 0
        else:
            self.counter = 0

    def start(self):
        self.last_timestamp = time.ticks_us()
        self.counter = 0
        self.pin.init(Pin.IN)
        self.pin.irq(self.rxinterrupt) # Defaults to falling and rising.

    def stop(self):
        self.pin.irq(None)

rc = recorder()
rc.start()