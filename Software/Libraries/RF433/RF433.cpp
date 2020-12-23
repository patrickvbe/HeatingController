#include "RF433.h"

struct Protocol
{
  int signallength;
  int relevantlength;
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
    Protocol{ 50,  48, 0, 0, 350, 1000, 1000,  350, 350, 10100},    // Elro
    Protocol{ 50,  48, 0, 0, 500,  500,  500, 1000, 500, 10000}     // PUMP_CONTROLLER, my own :-)
};
const int MAX_SIGNAL_LENGTH = 140;

sender::sender(const int sndpin)
{
  this->pin = sndpin;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void sender::send(const int protocol_id, const unsigned long code, int repeat)
{
  const Protocol& protocol = PROTOCOLS[protocol_id];
  while (repeat-- > 0)
  {
    int bitnr = (protocol.signallength / 2 - 1);
    unsigned long bitmask = 1UL << (bitnr - 1);
    while (bitnr-- > 0)
    {
      digitalWrite(pin, HIGH);
      if (code & bitmask)
      {
        //Serial.print("1");
        delayMicroseconds(protocol.one_high);
        digitalWrite(pin, LOW);
        delayMicroseconds(protocol.one_low);
      }
      else
      {
        //Serial.print("0");
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
    //Serial.println("");
  }
}

receiver* receiver::me = 0;

void receiver::initvalues()
{
  counter = 0;
  startpos = 0;
  nextpos = 1;
  data[startpos] = 0;
  last_timestamp = micros();
}

receiver::receiver(const int recpin)
{
  initvalues();
  pin = recpin;
  pinMode(pin, INPUT);
  receiver::me = this;
}

boolean receiver::receive(int& protocol, unsigned long& code)
{
  boolean result = false;
  if ( !result && data[startpos] != 0 )
  {
    /* Debug code to analyse new signals.
    auto pos = startpos;
    int size = (int)data[pos++];
    Serial.print(size);
    Serial.print(": ");
    for (int idx=0; idx < size; idx++)
    {
      Serial.print(data[pos++]);
      Serial.print(", ");
    }
    Serial.println();
    //*/
    result = decode(protocol, code);
  }
  return result;
}

// weatherstation format: 1111111 000000010100101 111110011011, middle part is temperature in 10ths C
int receiver::convertCodeToTemp(const unsigned long code)
{
  int temp = (code >> 12) & 0xFFF;
  if ( temp & 0x800 ) temp |= 0xF000; // Add missing negative bits.
  return temp;
}

void receiver::start()
{
  initvalues();
  attachInterrupt(digitalPinToInterrupt(pin), sread_interrupt, CHANGE);
}

void receiver::stop()
{
  detachInterrupt(digitalPinToInterrupt(pin));
}

ICACHE_RAM_ATTR void receiver::sread_interrupt()
{
  if ( (me->startpos - me->nextpos) >= 2 )  // Enough room for at least 2 values.
  {
    receiver::me->read_interrupt();
  }
}

bool receiver::decode(int& protocol, unsigned long& code)
{
  bool result = false;
  for (int idx = 0; !result && idx < (sizeof(PROTOCOLS) / sizeof(Protocol)); idx++)
  {
    if (decodeprotocol(idx, code))
    {
      protocol = idx;
      result = true;
    }
  }
  startpos += (int)data[startpos] + 1;
  return result;
}

bool receiver::decodeprotocol(const int protocol_id, unsigned long& result_code)
{
  const Protocol& protocol = PROTOCOLS[protocol_id];
  auto pos(startpos);
  const int size = (int)data[pos++];
  if (size < protocol.signallength)
    return false;
  pos += size - protocol.signallength;
  if (protocol.start_low != 0)
  {
    if (data[pos + 1] < (protocol.start_low * 2) / 3)
    {
      return false;
    }
    pos += 2;
  }
  if (protocol.end_low < (data[pos +  (protocol.signallength - 1)] * 2) / 3)
  {
    return false;
  }
  // pre-calculate ranges for speed...
  unsigned long zerohl = (protocol.zero_high / 3) * 2;
  unsigned long zerohh = zerohl * 2;
  unsigned long zeroll = (protocol.zero_low / 3) * 2;
  unsigned long zerolh = zeroll * 2;
  unsigned long onehl = (protocol.one_high / 3) * 2;
  unsigned long onehh = onehl * 2;
  unsigned long onell = (protocol.one_low / 3) * 2;
  unsigned long onelh = onell * 2;
  unsigned long code = 0;
  for (int idx = 0; idx < protocol.relevantlength; idx += 2)
  {
    unsigned long high = data[pos++];
    unsigned long low = data[pos++];
    if (zerohl < high && high < zerohh && zeroll < low && low < zerolh)
    {
      code <<= 1;
    }
    else if (onehl < high && high < onehh && onell < low && low < onelh)
    {
      code <<= 1;
      code |= 1UL;
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
  result_code = code;
  return true;
}

ICACHE_RAM_ATTR void receiver::read_interrupt()
{
  unsigned long timestamp = micros();
  unsigned long duration = timestamp - last_timestamp;
  last_timestamp = timestamp;

  // Pulses shorter than 250us are usually noise. But my smartwares remote drifts to almost 225us...
  if (duration > DURATIONTHRESHOLD1 || (duration > DURATIONTHRESHOLD2 && counter > 0))
  {
    counter++;
    data[nextpos++] = duration;
    if (duration > SYNCPULSETHRESHOLD || counter == MAX_SIGNAL_LENGTH)
    {
      if (counter > 10)
      {
        data[nextpos - (counter + 1)] =  counter;
        data[nextpos++] = 0;
      }
      else
      {
        nextpos -= counter;
      }
      
      counter = 0;
    }
  }
  else
  {
    nextpos -= counter;
    counter = 0;
  }
}
