struct Protocol {
    byte signallength;
    byte relevantlength;
    unsigned int start_high;
    unsigned int start_low,
    unsigned int zero_high;
    unsigned int zero_low;
    unsigned int one_high;
    unsigned int one_low;
    unsigned int end_high
    unsigned int end_low;
}

const int SMARTWARES     = 0;
const int WEATHERSTATION = 1;
const int CONRADRSL      = 2;
const int ELRO           = 3;

const Protocol PROTOCOLS[] = {
    //Protocol(SMARTWARES,    132, 130, 300, 2500, 300,  300,  300, 1200, 300, 10000),     // Smartwares (Action)
    {Protocol(130, 128,   0,    0, 300,  300,  300, 1200, 300, 10000)},   // Smartwares (Action)
    {Protocol(74,  70,   0,    0, 525,  925,  525, 1850, 550,  3850)},   // Weatherstation
    {Protocol(66,  64,   0,    0, 600, 1200, 1200,  600, 600,  7000)},   // Conrad RSL
    {Protocol(50,  48,   0,    0, 325,  975,  975,  325, 325, 10000)}    // Elro
};

// The first threshold is when we start recording, the second is the smalles valid value while recording.
// The Smartwares transmitter is a nightmare. It's pulses are often so short they blend in with the background noise...
// If you are not using some protocols with low timing, you can set the thresholds to a higher value to lower the CPU load.
// MAXRECORD is the maximum buffer size.
// const unsigned long DURATIONTHRESHOLD1 = 275;
// const unsigned long DURATIONTHRESHOLD2 = 225;
const unsigned long DURATIONTHRESHOLD1 = 250;
const unsigned long DURATIONTHRESHOLD2 = 225;
const unsigned long SYNCPULSETHRESHOLD =3000;
const int           MAXRECORD 150;

// weatherstation format: 1111111 000000010100101 111110011011, middle part is temperatur in 10ths C
int getTempFromWeather(const int weather):
{
    return (weather >> 12) & 0xFFF;
}

class sender {
  private:
    int pin;

  public:  
    void sender(const int pin)
    {
        this.pin = pin;
        // Decouple...
        pinMode(this.pin, INPUT);
    }

    void send(const byte protocol_id, const unsigned long code, int repeat = 3)
    {
        const Protocol& protocol = PROTOCOLS[protocol_id];
        while (repeat-- > 0)
        {
            pinMode(this.pin, OUTPUT);
            int bitnr = (protocol.signallength); // 2 - 1
            unsigned long bitmask = 1 << (bitnr - 1);
            while (bitnr-- > 0)
            {
                digitalWrite(this.pin, HIGH);
                if (code & bitmask == 0)
                {
                    delayMicroseconds(protocol.zero_high);
                    digitalWrite(this.pin, LOW);
                    delayMicroseconds(protocol.zero_low);
                }
                else
                {
                    delayMicroseconds(protocol.one_high);
                    digitalWrite(this.pin, LOW);
                    delayMicroseconds(protocol.one_low);
                }
                bitmask >>= 1
            }
            digitalWrite(this.pin, HIGH);
            delayMicroseconds(protocol.end_high);
            digitalWrite(this.pin, LOW);
            delayMicroseconds(protocol.end_low);
            // Decouple...
            pinMode(this.pin, INPUT);
            if (repeat != 0) delay(10);
        }
    }
}

class receiver
{
  public:
    typedef void (received_callback*)(int protocol, unsigned long code, unsigned long timestamp);

  private:
    int                 pin;
    int                 counter;
    unsigned long       data[MAXRECORD];
    unsigned long       last_timestamp;
    received_callback   callback;
    static receiver*    me = nullptr;
    
  public:

    void receiver(int pin, received_callback callback = nullptr)
    {
        this.last_timestamp = micros();
        this.counter = 0;
        this.callback = callback;
        this.pin = pin;
        pinMode(this.pin, INPUT);
        reciever.me = this;
    }

    void start()
    {
        this.last_timestamp = micros();
        this.counter = 0;
        attachInterrupt(this.pin, read_interrupt, CHANGE);
    }

    void stop()
    {
        detachInterrupt(this.pin);        
    }

  private:
    bool decode()
    {
        for (idx = 0; idx < sizeof(PROTOCOLS); idx++)
            if (this.decodeprotocol(idx)) return true;
        return false;
    }

    bool decodeprotocol(const int protocol_id)
    {
        const Protocol* protocol = PROTOCOLS[protocol_id];
        if ( this.counter < protocol.signallength ) return false;
        int start = this.counter - protocol.signallength;
        if ( protocol.start_low != 0 )
        {
            if ( this.data[start + 1] < (protocol.start_low * 2) / 3 )
                return false;
            start += 2;
        }
        if ( protocol.end_low < (this.data[this.counter - 1] * 2) / 3 )
            return false;
        // pre-calculate ranges for speed...
        unsigned long zerohl = (protocol.zero_high) / 2;
        unsigned long zerohh = zerohl * 3;
        unsigned long zeroll = (protocol.zero_low) / 2;
        unsigned long zerolh = zeroll * 3;
        unsigned long onehl = (protocol.one_high) / 2;
        unsigned long onehh = onehl * 3;
        unsigned long onell = (protocol.one_low) / 2;
        unsigned long onelh = onell * 3;
        unsigned long code = 0;
        for pos in range(start, start + protocol.relevantlength, 2):
            high = this.data[pos]
            low = this.data[pos+1]
            if zerohl < high and  high < zerohh and zeroll < low and low < zerolh:
                code <<= 1
            elif onehl < high and  high < onehh and onell < low and low < onelh:
                code <<= 1
                code |= 1
            else:
                return False
        if code == 0:
            return False
        // Pass it to the user.
        this.code = code
        this.code_timestamp = this.last_timestamp
        this.code_protocol = protocol.id
        if this._callback:
            this._callback(this)
        return True
    }

    static void read_interrupt(this, pin)
    {
        receiver* myself = receiver::me;
        timestamp = micros()
        duration = time.ticks_diff(timestamp, myself.last_timestamp)
        myself.last_timestamp = timestamp

        // Pulses shorter than 250us are usually noise. But my smartwares remote drifts to almost 225us...
        if ( duration > DURATIONTHRESHOLD1 or (duration > DURATIONTHRESHOLD2 and myself.counter > 0) )
        {
            myself.data[this.counter] = duration
            myself.counter += 1
            if duration > SYNCPULSETHRESHOLD or myself.counter == MAXRECORD
            {
                // this.led.value(not this.led.value())
                myself.decode();
                myself.counter = 0
            }
        }
        else:
            this.counter = 0
    }
}