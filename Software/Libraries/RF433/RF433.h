#include <Arduino.h>

const int SMARTWARES = 0;
const int WEATHERSTATION = 1;
const int CONRADRSL = 2;
const int ELRO = 3;
const int PUMP_CONTROLLER = 4;

// The first threshold is when we start recording, the second is the smalles valid value while recording.
// The Smartwares transmitter is a nightmare. It's pulses are often so short they blend in with the background noise...
// If you are not using some protocols with low timing, you can set the thresholds to a higher value to lower the CPU load.
// MAXRECORD is the maximum buffer size.
const unsigned long DURATIONTHRESHOLD1 = 275;
const unsigned long DURATIONTHRESHOLD2 = 225;
const unsigned long SYNCPULSETHRESHOLD = 3000;
const int BUFFER_SIZE = 1000;

template<class T, T OVERFLOW_VALUE>
class overflowcounter final
{
  public:
    overflowcounter() : counter(0) {}
    overflowcounter(T value) : counter(value) {}
    operator T() { return counter; }
    overflowcounter operator=(T that) { return counter = that; }
    overflowcounter operator++(T)
    { 
      T retvalue = counter;
      (*this)+=1;
      return retvalue;
    }
    overflowcounter operator-(T that) const
    { 
      return counter >= that ? counter - that : counter + OVERFLOW_VALUE - that;
    }
    overflowcounter operator-=(T that)
    { 
      return counter = (*this) - that;
    }
    overflowcounter operator+(T that) const
    { 
      T result = counter + that;
      return result >= OVERFLOW_VALUE ? result - OVERFLOW_VALUE : result;
    }
    overflowcounter operator+=(T that)
    { 
      return counter = (*this) + that;
    }

  private:
    T counter = 0;
};

class sender
{
public:
// Constructors / destructor
  sender(const int sndpin);

// Functionality
  void send(const int protocol_id, const unsigned long code, int repeat = 6);

// Data members
  int pin;
};

/* Receive 433MHz codes. Although everything is packed in a class, the interrupt code
   basically allows only one instance (why would you use multiple anyway?)
*/
class receiver
{
public:
// Constructors / destructor
  receiver(const int recpin);

// Functionality
  void start();
  void stop();
  boolean receive(int& protocol, unsigned long& code);
  static int convertCodeToTemp(const unsigned long code);

// Data members
  int pin;

private:
// Functionality
  static void sread_interrupt();
  void read_interrupt();
  bool decode(int& protocol, unsigned long& code);
  bool decodeprotocol(const int protocol_id, unsigned long& code);
  void initvalues();

// Data members
  overflowcounter<int, BUFFER_SIZE> startpos; // Should be shifted up by the processor
  overflowcounter<int, BUFFER_SIZE> nextpos;
  int            counter;
  unsigned long  data[BUFFER_SIZE];
  unsigned long  last_timestamp;
  static receiver* me;  // No arguments allowed for the interrupt, so: it's me!
};
