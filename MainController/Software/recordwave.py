import time
import micropython
from machine import Pin
import array

micropython.alloc_emergency_exception_buf(500)

class recorder:
    MAXRECORD = 500
    data = array.array('L', (0,) * MAXRECORD)
    pin = Pin(5)
    led = Pin(2)
    led.init(Pin.OUT)
    last_timestamp = time.ticks_us()
    counter = 0
    highcount = 0

    def rxinterrupt(self, pin):
        timestamp = time.ticks_us()
        duration = time.ticks_diff(timestamp, self.last_timestamp)
        self.last_timestamp = timestamp

        # Pulses shorter than 250 are usually noise. Though it might happen...
        if duration > 225:
            self.data[self.counter] = duration
            self.counter += 1
            if duration > 300:
                self.highcount += 1
            if duration > 4000 or self.counter == self.MAXRECORD:
                self.led.value(not self.led.value())
                if self.highcount * 5 > self.counter and self.counter > 25:
                    # at least 20% highs to be a signal instead of noise
                    print(self.counter, self.data[0:self.counter])
                self.counter = 0
                self.highcount = 0
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