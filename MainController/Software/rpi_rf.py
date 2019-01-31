"""
Sending and receiving 433/315Mhz signals with low-cost GPIO RF Modules on ESP.
"""

import time
import micropython
from collections import namedtuple
from machine import Pin
from copy import copy

micropython.alloc_emergency_exception_buf(100)

MAX_CHANGES = micropython.const(67)

Protocol = namedtuple('Protocol',
                      ('pulselength',
                       'sync_high', 'sync_low',
                       'zero_high', 'zero_low',
                       'one_high', 'one_low'))
PROTOCOLS = (None,
             Protocol(350, 1, 31, 1, 3, 3, 1),
             Protocol(650, 1, 10, 1, 2, 2, 1),
             Protocol(100, 30, 71, 4, 11, 9, 6),
             Protocol(380, 1, 6, 1, 3, 3, 1),
             Protocol(500, 6, 14, 1, 2, 2, 1),
             Protocol(200, 1, 10, 1, 5, 1, 1))


class RFCode:
    def __init__(self):
        self.code = None

class RFDevice:
    """Representation of a GPIO RF device."""

    # pylint: disable=too-many-instance-attributes,too-many-arguments
    def __init__(self, pin, readcallback=None,
                 tx_proto=1, tx_pulselength=None, tx_repeat=10, tx_length=24, rx_tolerance=80):
        """Initialize the RF device."""
        self.pin = pin
        self.readcallback = readcallback
        self.tx_enabled = False
        self.tx_proto = tx_proto
        if tx_pulselength:
            self.tx_pulselength = tx_pulselength
        else:
            self.tx_pulselength = PROTOCOLS[tx_proto].pulselength
        self.tx_repeat = tx_repeat
        self.tx_length = tx_length
        self.rx_enabled = False
        self.rx_tolerance = rx_tolerance
        # internal values
        self._rx_timings = [0] * (MAX_CHANGES + 1)
        self._rx_last_timestamp = 0
        self._rx_change_count = 0
        self._rx_repeat_count = 0
        # successful RX value, pre-allocate in order to use it from the interrupt.
        self.readcode = RFCode()

    def DoCallback(self):
        # We make a copy here, we can't allocate in the interrupt handler. Then we do the actual
        # callback with the copy, so the interrupt code can re-use the pre-allocated version.
        # This way, it doesn't matter if the callback takes more time because it won't be using
        # the original.
        print("received", self.readcode.code, self.readcode.timestamp, self.readcode.bitlength, self.readcode.pulselength, self.readcode.proto)
        if self.readcallback:
            self.readcallback(copy(self.readcode))

    def cleanup(self):
        """Disable TX and RX and clean up GPIO."""
        if self.tx_enabled:
            self.disable_tx()
        if self.rx_enabled:
            self.disable_rx()

    def enable_tx(self):
        """Enable TX, set up GPIO."""
        if self.rx_enabled:
            return False
        if not self.tx_enabled:
            self.tx_enabled = True
            self.pin.init(Pin.OUT)
        return True

    def disable_tx(self):
        """Disable TX, reset GPIO."""
        if self.tx_enabled:
            # set up GPIO pin as input for safety
            self.pin.init(Pin.IN)
            self.tx_enabled = False
        return True

    def tx_code(self, code, tx_proto=None, tx_pulselength=None, tx_length=None):
        """
        Send a decimal code.

        Optionally set protocol, pulselength and code length.
        When none given reset to default protocol, default pulselength and set code length to 24 bits.
        """
        if tx_proto:
            self.tx_proto = tx_proto
        else:
            self.tx_proto = 1
        if tx_pulselength:
            self.tx_pulselength = tx_pulselength
        elif not self.tx_pulselength:
            self.tx_pulselength = PROTOCOLS[self.tx_proto].pulselength
        if tx_length:
            self.tx_length = tx_length
        elif self.tx_proto == 6:
            self.tx_length = 32
        elif (code > 16777216):
            self.tx_length = 32
        else:
            self.tx_length = 24
        rawcode = format(code, '#0{}b'.format(self.tx_length + 2))[2:]
        if self.tx_proto == 6:
            nexacode = ""
            for b in rawcode:
                if b == '0':
                    nexacode = nexacode + "01"
                if b == '1':
                    nexacode = nexacode + "10"
            rawcode = nexacode
            self.tx_length = 64
        return self.tx_bin(rawcode)

    def tx_bin(self, rawcode):
        """Send a binary code."""
        for _ in range(0, self.tx_repeat):
            if self.tx_proto == 6:
                if not self.tx_sync():
                    return False
            for byte in range(0, self.tx_length):
                if rawcode[byte] == '0':
                    if not self.tx_l0():
                        return False
                else:
                    if not self.tx_l1():
                        return False
            if not self.tx_sync():
                return False

        return True

    def tx_l0(self):
        """Send a '0' bit."""
        if not 0 < self.tx_proto < len(PROTOCOLS):
            return False
        return self.tx_waveform(PROTOCOLS[self.tx_proto].zero_high,
                                PROTOCOLS[self.tx_proto].zero_low)

    def tx_l1(self):
        """Send a '1' bit."""
        if not 0 < self.tx_proto < len(PROTOCOLS):
            return False
        return self.tx_waveform(PROTOCOLS[self.tx_proto].one_high,
                                PROTOCOLS[self.tx_proto].one_low)

    def tx_sync(self):
        """Send a sync."""
        if not 0 < self.tx_proto < len(PROTOCOLS):
            return False
        return self.tx_waveform(PROTOCOLS[self.tx_proto].sync_high,
                                PROTOCOLS[self.tx_proto].sync_low)

    def tx_waveform(self, highpulses, lowpulses):
        """Send basic waveform."""
        if not self.tx_enabled:
            return False
        self.pin.on()
        time.sleep_us(highpulses * self.tx_pulselength)
        self.pin.off()
        time.sleep_us(lowpulses * self.tx_pulselength)
        return True

    def enable_rx(self):
        """Enable RX, set up GPIO and add event detection."""
        if self.tx_enabled:
            return False
        if not self.rx_enabled:
            self.rx_enabled = True
            self.pin.init(Pin.IN)
            self.pin.irq(self.rx_callback) # Defaults to falling and rising.
        return True

    def disable_rx(self):
        """Disable RX, remove GPIO event detection."""
        if self.rx_enabled:
            self.pin.irq()
            self.rx_enabled = False
        return True

    def rx_callback(self, pin):
        """RX callback for GPIO event detection. Handle basic signal detection."""
        timestamp = time.ticks_us()
        duration = time.ticks_diff(timestamp - self._rx_last_timestamp)

        if duration > 5000:
            if abs(duration - self._rx_timings[0]) < 200:
                self._rx_repeat_count += 1
                self._rx_change_count -= 1
                if self._rx_repeat_count == 2:
                    for pnum in range(1, len(PROTOCOLS)):
                        if self._rx_waveform(pnum, self._rx_change_count, timestamp):
                            micropython.schedule(self.DoCallback)
                            break
                    self._rx_repeat_count = 0
            self._rx_change_count = 0

        if self._rx_change_count >= MAX_CHANGES:
            self._rx_change_count = 0
            self._rx_repeat_count = 0
        self._rx_timings[self._rx_change_count] = duration
        self._rx_change_count += 1
        self._rx_last_timestamp = timestamp

    def _rx_waveform(self, pnum, change_count, timestamp):
        """Detect waveform and format code."""
        code = 0
        delay = int(self._rx_timings[0] / PROTOCOLS[pnum].sync_low)
        delay_tolerance = delay * self.rx_tolerance / 100

        for i in range(1, change_count, 2):
            if (abs(self._rx_timings[i] - delay * PROTOCOLS[pnum].zero_high) < delay_tolerance and
                abs(self._rx_timings[i+1] - delay * PROTOCOLS[pnum].zero_low) < delay_tolerance):
                code <<= 1
            elif (abs(self._rx_timings[i] - delay * PROTOCOLS[pnum].one_high) < delay_tolerance and
                  abs(self._rx_timings[i+1] - delay * PROTOCOLS[pnum].one_low) < delay_tolerance):
                code <<= 1
                code |= 1
            else:
                return False

        if self._rx_change_count > 6 and code != 0:
            self.readcode.code = code
            self.readcode.timestamp = timestamp
            self.readcode.bitlength = int(change_count / 2)
            self.readcode.pulselength = delay
            self.readcode.proto = pnum
            return True

        return False
           