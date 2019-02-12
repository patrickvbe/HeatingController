#include "RF433.h"

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

static const Protocol PROTOCOLS[] = {
    //Protocol{132, 130, 300, 2500, 300,  300,  300, 1200, 300, 10000},     // Smartwares (Action)
    Protocol{130, 128, 0, 0, 300,  300,  300, 1200, 300, 10000},    // Smartwares (Action)
    Protocol{ 74,  70, 0, 0, 500,  950,  500, 1950, 550,  3850},    // Weatherstation
    Protocol{ 66,  64, 0, 0, 600, 1200, 1200,  600, 600,  7000},    // Conrad RSL
    Protocol{ 50,  48, 0, 0, 400,  900, 1050,  250, 400, 10100},    // Elro
    Protocol{ 13,  12, 0, 0, 500,  500,  500, 1000, 400, 10000}     // PUMP_CONTROLLER, my own :-)
};


sender::sender(const int sndpin)
{
  this->pin = sndpin;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void sender::send(const int protocol_id, const unsigned long code, int repeat)
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
    if ( protocol.end_low > 2000 ) delay(protocol.end_low / 1000);
    else delayMicroseconds(protocol.end_low);
  }
}

receiver* receiver::me = 0;

receiver::receiver(const int recpin, received_callback reccallback)
{
  last_timestamp = micros();
  counter = 0;
  callback = reccallback;
  pin = recpin;
  pinMode(pin, INPUT);
  receiver::me = this;
}

// weatherstation format: 1111111 000000010100101 111110011011, middle part is temperatur in 10ths C
int receiver::convertCodeToTemp(const unsigned long code)
{
  int temp = (code >> 12) & 0xFFF;
  if ( temp & 0x800 ) temp |= 0xF000; // Add missing negative bits.
  return temp;
}

void receiver::start()
{
  last_timestamp = micros();
  counter = 0;
  attachInterrupt(pin, sread_interrupt, CHANGE);
}

void receiver::stop()
{
  detachInterrupt(pin);
}

void receiver::sread_interrupt()
{
  receiver::me->read_interrupt();
}

bool receiver::decode()
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

bool receiver::decodeprotocol(const int protocol_id)
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

void receiver::read_interrupt()
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
        /* Debug code to analyse new signals.
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
