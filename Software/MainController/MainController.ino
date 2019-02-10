#include "RF433.h"

#define DEBUG;
#ifdef DEBUG
  #define DEBUGONLY(statement) statement;
#else
  #define DEBUGONLY()
#endif

#define SENDER_PIN   4
#define RECEIVER_PIN 5

#define INVALID_TEMP  0xFFFF
#define TEMP_VALIDITY 300000

int           outsideTemperature = INVALID_TEMP;
int           outsidePreviousTemperature = INVALID_TEMP;
unsigned long outsideTimestamp  = 0; // millis
int           insideTemperature  = INVALID_TEMP;

void code_received(int protocol, unsigned long code, unsigned long timestamp)
{
  if ( protocol == WEATHERSTATION)
  {
    outsideTemperature = receiver::convertCodeToTemp(code);
    outsideTimestamp = millis();
  }
  // Serial.print("Protocol: ");
  // Serial.print(protocol);
  // Serial.print(", code:");
  // Serial.print(code, BIN);
  // Serial.print(" / ");
  // Serial.print(code, HEX);
  // if ( protocol == WEATHERSTATION)
  // {
  //   Serial.print(", T: ");
  //   Serial.print(receiver::convertCodeToTemp(code));
  // }
  // Serial.println();
}

receiver rcv(RECEIVER_PIN, code_received);
sender   snd(SENDER_PIN);

void setup()
{
  Serial.begin(230400);
  rcv.start();
}

void loop()
{
  unsigned long timestamp = millis();

  // outside temperature
  if ( timestamp - outsideTimestamp > TEMP_VALIDITY)
  {
    outsideTemperature = INVALID_TEMP;
  }
  if ( outsidePreviousTemperature != outsideTemperature )
  {
    DEBUGONLY(Serial.print("Outside temp changed to "));
    DEBUGONLY(Serial.println(outsideTemperature));
    outsidePreviousTemperature = outsideTemperature;
    // TODO: Update screen here.
  }

  // Test code
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
