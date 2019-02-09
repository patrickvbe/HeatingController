

struct Protocol
{
  byte signallength;
  byte relevantlength;
  unsigned int start_high;
  unsigned int start_low;
  unsigned int zero_high;
  unsigned int zero_low;
  unsigned int one_high;
  unsigned int one_low;
  unsigned int end_high;
  unsigned int end_low;
};

const int SMARTWARES = 0;
const int WEATHERSTATION = 1;
const int CONRADRSL = 2;
const int ELRO = 3;

const Protocol PROTOCOLS[] = {
    //Protocol{132, 130, 300, 2500, 300,  300,  300, 1200, 300, 10000},     // Smartwares (Action)
    Protocol{130, 128, 0, 0, 300,  300,  300, 1200, 300, 10000},    // Smartwares (Action)
    Protocol{ 74,  70, 0, 0, 500,  950,  500, 1950, 550,  3850},    // Weatherstation
    Protocol{ 66,  64, 0, 0, 600, 1200, 1200,  600, 600,  7000},    // Conrad RSL
    //Protocol{ 50,  48, 0, 0, 300, 1000, 1000,  300, 300, 10000}     // Elro
    Protocol{ 50,  48, 0, 0, 400,  900, 1050,  250, 400, 10100}     // Elro
/*
*/
};

// The first threshold is when we start recording, the second is the smalles valid value while recording.
// The Smartwares transmitter is a nightmare. It's pulses are often so short they blend in with the background noise...
// If you are not using some protocols with low timing, you can set the thresholds to a higher value to lower the CPU load.
// MAXRECORD is the maximum buffer size.
// const unsigned long DURATIONTHRESHOLD1 = 275;
// const unsigned long DURATIONTHRESHOLD2 = 225;
const unsigned long DURATIONTHRESHOLD1 = 275;
const unsigned long DURATIONTHRESHOLD2 = 225;
const unsigned long SYNCPULSETHRESHOLD = 3000;
const int MAXRECORD = 150;

// weatherstation format: 1111111 000000010100101 111110011011, middle part is temperatur in 10ths C
int getTempFromWeather(const unsigned long weather)
{
  int temp = (weather >> 12) & 0xFFF;
  if ( temp & 0x800 ) temp |= 0xF000; // Add missing negative bits.
  return temp;
}

class sender
{
public:
  int pin;

  sender(const int sndpin)
  {
    this->pin = sndpin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  void send(const int protocol_id, const unsigned long code, int repeat = 6)
  {
    const Protocol &protocol = PROTOCOLS[protocol_id];
    while (repeat-- > 0)
    {
      int bitnr = (protocol.signallength / 2 - 1);
      unsigned long bitmask = 1 << (bitnr - 1);
      while (bitnr-- > 0)
      {
        digitalWrite(pin, HIGH);
        if (code & bitmask)
        {
          delayMicroseconds(protocol.one_high);
          digitalWrite(pin, LOW);
          delayMicroseconds(protocol.one_low);
        }
        else
        {
          delayMicroseconds(protocol.zero_high);
          digitalWrite(pin, LOW);
          delayMicroseconds(protocol.zero_low);
        }
        bitmask >>= 1;
      }
      digitalWrite(pin, HIGH);
      delayMicroseconds(protocol.end_high);
      digitalWrite(pin, LOW);
      delayMicroseconds(protocol.end_low);
    }
  }
};

/* Receive 433MHz codes. Although everything is packed in a class, the interrupt code
   basically allows only one instance (why would you use multiple anyway?)
*/
class receiver
{
public:


  typedef void(*received_callback)(int protocol, unsigned long code, unsigned long timestamp);
  int pin;
  // This is where the code will be (usefull if you don't use the callback)
  unsigned long code = 0;
  unsigned long code_timestamp = 0;
  int           code_protocol = -1;

private:
  int counter;
  unsigned long data[MAXRECORD];
  unsigned long last_timestamp;
  received_callback callback;
  static receiver* me;  // No arguments allowed for the interrupt, so: it's me!

public:
  receiver(const int recpin, received_callback reccallback = 0)
  {
    last_timestamp = micros();
    counter = 0;
    callback = reccallback;
    pin = recpin;
    pinMode(pin, INPUT);
    receiver::me = this;
  }

  static void _read_interrupt()
  {
    receiver::me->read_interrupt();
  }

  void start()
  {
    last_timestamp = micros();
    counter = 0;
    attachInterrupt(pin, _read_interrupt, CHANGE);
  }

  void stop()
  {
    detachInterrupt(pin);
  }

private:
  bool decode()
  {
    for (int idx = 0; idx < sizeof(PROTOCOLS); idx++)
    {
      if (decodeprotocol(idx))
      {
        return true;
      }
    }
    return false;
  }

  bool decodeprotocol(const int protocol_id)
  {
    const Protocol& protocol = PROTOCOLS[protocol_id];
    if (counter < protocol.signallength)
      return false;
    int start = counter - protocol.signallength;
    if (protocol.start_low != 0)
    {
      if (data[start + 1] < (protocol.start_low * 2) / 3)
      {
        return false;
      }
      start += 2;
    }
    if (protocol.end_low < (data[counter - 1] * 2) / 3)
    {
      return false;
    }
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
    int end = start + protocol.relevantlength;
    for (int pos = start; pos < end; pos += 2)
    {
      unsigned long high = data[pos];
      unsigned long low = data[pos + 1];
      if (zerohl < high && high < zerohh && zeroll < low && low < zerolh)
      {
        code <<= 1;
      }
      else if (onehl < high && high < onehh && onell < low && low < onelh)
      {
        code <<= 1;
        code |= 1;
      }
      else
      {
        return false;
      }
    }
    if (code == 0)
    {
      return false;
    }
    // Pass it to the user.
    code = code;
    code_timestamp = last_timestamp;
    code_protocol = protocol_id;
    if (callback)
    {
      callback(protocol_id, code, code-code_timestamp);
    }
    return true;
  }

  void read_interrupt()
  {
    unsigned long timestamp = micros();
    unsigned long duration = timestamp - last_timestamp;
    last_timestamp = timestamp;

    // Pulses shorter than 250us are usually noise. But my smartwares remote drifts to almost 225us...
    if (duration > DURATIONTHRESHOLD1 or (duration > DURATIONTHRESHOLD2 and counter > 0))
    {
      data[counter] = duration;
      counter += 1;
      if (duration > SYNCPULSETHRESHOLD or counter == MAXRECORD)
      {
        if (counter > 10)
        {
          /*
          Serial.print(counter);
          Serial.print(": ");
          for (int idx=0; idx < counter; idx++)
          {
            Serial.print(data[idx]);
            Serial.print(", ");
          }
          Serial.println();
          //*/
          decode();
        }
        counter = 0;
      }
    }
    else
    {
      counter = 0;
    }
  }
};

receiver* receiver::me = 0;

//--------------------------------------------------

void code_received(int protocol, unsigned long code, unsigned long timestamp)
{
  Serial.print("Protocol: ");
  Serial.print(protocol);
  Serial.print(", code:");
  Serial.print(code, BIN);
  Serial.print(" / ");
  Serial.print(code, HEX);
  if ( protocol == WEATHERSTATION)
  {
    Serial.print(", T: ");
    Serial.print(getTempFromWeather(code));
  }
  Serial.println();
}

receiver rcv(5, code_received);
sender   snd(4);

void setup()
{
  Serial.begin(230400);
  rcv.start();
}

void loop()
{
  int cin = Serial.read();
  if (cin == '1')
  {
    rcv.stop();
    snd.send(ELRO, 0x144551);
    rcv.start();
  }
  else if (cin == '0')
  {
    rcv.stop();
    snd.send(ELRO, 0x144554);
    rcv.start();
  }
}
