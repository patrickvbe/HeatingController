import time
import micropython
from collections import namedtuple
from machine import Pin
import array

micropython.alloc_emergency_exception_buf(100)

Protocol = namedtuple('Protocol',
                      ('id', 'signallength', 'relevantlength',
                       'start_high', 'start_low',
                       'zero_high', 'zero_low',
                       'one_high', 'one_low',
                       'end_high', 'end_low'))

SMARTWARES = micropython.const(1)
WEATHERSTATION = micropython.const(2)
CONRADRSL = micropython.const(3)
ELRO = micropython.const(4)

PROTOCOLS = micropython.const((
    #Protocol(SMARTWARES,    132, 130, 300, 2500, 300,  300,  300, 1200, 300, 10000),     # Smartwares (Action)
    Protocol(SMARTWARES,    130, 128,   0,    0, 300,  300,  300, 1200, 300, 10000),     # Smartwares (Action)
    Protocol(WEATHERSTATION, 74,  70,   0,    0, 525,  925,  525, 1850, 550,  3850),     # Weatherstation
    Protocol(CONRADRSL,      66,  64,   0,    0, 600, 1200, 1200,  600, 600,  7000),     # Conrad RSL
    Protocol(ELRO,           50,  48,   0,    0, 325,  975,  975,  325, 325, 10000)      # Elro
))

# The first thrashold is when we start recording, the second is the smalles valid value while recording.
# The Smartwares transmitter is a nightmare. It's pulses are often so short they blend in with the background noise...
# If you are not using some protocols with low timing, you can set the trasholds to a higher value to lower the CPU load.
# MAXRECORD is the maximum buffer size.
# DURATIONTRASHOLD1 = micropython.const(275)
# DURATIONTRASHOLD2 = micropython.const(225)
DURATIONTRASHOLD1 = micropython.const(275)
DURATIONTRASHOLD2 = micropython.const(275)
SYNCPULSETRASHOLD = micropython.const(3000)
MAXRECORD = micropython.const(150)

# weatherstation format: 1111111 000000010100101 111110011011, middle part is temperatur in 10ths C
def getTempFromWeather(weather):
    return (weather >> 12) & 0xFFF


class receiver:

    def __init__(self, pin, callback = None):
        self._data = array.array('L', (0,) * MAXRECORD)
        self._last_timestamp = time.ticks_us()
        self._counter = 0
        self._callback = callback
        self.pin = Pin(5)
        self.code = 0
        self.code_timestamp = 0
        self.code_protocol = 0
        # self.led = Pin(2)
        # self.led.init(Pin.OUT)

    def _decode(self):
        for protocol in PROTOCOLS:
            if self._decodeprotocol(protocol):
                return True
        return False

    def _decodeprotocol(self, protocol):
        if self._counter < protocol.signallength:
            return False
        start = self._counter - protocol.signallength
        if protocol.start_low != 0:
            if self._data[start + 1] < (protocol.start_low * 2) // 3:
                return False
            start += 2
        if protocol.end_low < (self._data[-1] * 2) // 3:
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
        code = 0
        for pos in range(start, start + protocol.relevantlength, 2):
            high = self._data[pos]
            low = self._data[pos+1]
            if zerohl < high and  high < zerohh and zeroll < low and low < zerolh:
                code <<= 1
            elif onehl < high and  high < onehh and onell < low and low < onelh:
                code <<= 1
                code |= 1
            else:
                return False
        if code == 0:
            return False
        # Pass it to the user.
        self.code = code
        self.code_timestamp = self._last_timestamp
        self.code_protocol = protocol.id
        if self._callback:
            self._callback(self)
        return True

    def _rxinterrupt(self, pin):
        timestamp = time.ticks_us()
        duration = time.ticks_diff(timestamp, self._last_timestamp)
        self._last_timestamp = timestamp

        # Pulses shorter than 250us are usually noise. But my smartwares remote drifts to almost 225us...
        if duration > DURATIONTRASHOLD1 or (duration > DURATIONTRASHOLD2 and self._counter > 0):
            self._data[self._counter] = duration
            self._counter += 1
            if duration > SYNCPULSETRASHOLD or self._counter == MAXRECORD:
                # self.led.value(not self.led.value())
                self._decode()
                self._counter = 0
        else:
            self._counter = 0

    def start(self):
        self._last_timestamp = time.ticks_us()
        self._counter = 0
        self.pin.init(Pin.IN)
        self.pin.irq(self._rxinterrupt) # Defaults to falling and rising.

    def stop(self):
        self.pin.irq(None)
