import time
import micropython
from machine import Pin
import array

micropython.alloc_emergency_exception_buf(100)

class recorder:
    MAXRECORD = 500
    data = array.array('L', (0,) * MAXRECORD)
    pin = Pin(5)
    last_timestamp = time.ticks_us()
    counter = 0
    recording = False
    samecount = 0

    def decodeElro(self):
        if self.counter < 50:
            return False
        short = self.data[self.counter - 2] # The short as part of the end-sync
        cutof = short * 2
        code = 0
        for count in range(self.counter - 50, self.counter - 2, 2):
            if self.data[count] <= cutof and self.data[count+1] > cutof:
               # zero
               code <<= 1
            elif self.data[count] > cutof and self.data[count+1] <= cutof:
               code <<= 1
               code |= 1
            else:
                print("failed", cutof, count, self.data[count], self.data[count+1], self.data[count+2], self.data[count+3])
                return False
        print("Decoded: {0} / 0x{0:x} / {0:b}".format(code))
        return True

    def rxinterrupt(self, pin):
        timestamp = time.ticks_us()
        duration = time.ticks_diff(timestamp, self.last_timestamp)
        self.last_timestamp = timestamp

        # Pulses shorter than 250 are usually noise. Though it might happen...
        if duration > 250:
            self.data[self.counter] = duration
            self.counter += 1
            if duration > 5000 or self.counter == self.MAXRECORD:
                if self.counter < 10:
                    pass
                elif self.decodeElro():
                    pass
                else:
                    print(self.counter, self.data[0:self.counter])
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