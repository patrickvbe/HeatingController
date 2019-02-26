#include <OneWire.h>
#include <DallasTemperature.h>
#include <RF433.h>
#include <InterUnitCommunication.h>

#define DEBUG;
#ifdef DEBUG
  #define DEBUGONLY(statement) statement;
#else
  #define DEBUGONLY()
#endif

#define PUMP_PIN   2
#define SENDER_PIN   4
#define RECEIVER_PIN 5
#define DS18B20_PIN  9

const int INVALID_TEMP =       -1000;
#define   TEMP_VALIDITY       300000
#define   MEASURE_INTERVAL     10000
#define   FORCE_TIME_DURATION 300000

// The values we preserve
InterUnitCommunication  masterCommand(0);
unsigned long masterCommandTimestamp = 0;
unsigned long masterReceived = 0;
unsigned long masterReceivedTimestamp = 0;
int           masterForceTime = 0;  // End time in ms
int           waterTemperature  = INVALID_TEMP;
unsigned long waterTimestamp = 0;

// Interrupt routine for RF433 communication.
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
  // Serial.println();
}

// The objects / sensors we have
receiver          rcv(RECEIVER_PIN, code_received);
sender            snd(SENDER_PIN);
DallasTemperature watertemp(&onewire);

void setup()
{
  Serial.begin(230400);
  masterCommand.temperature = INVALID_TEMP;
  rcv.start();
  watertemp.begin();
  watertemp.setResolution(10);
}

void loop()
{
  unsigned long timestamp = millis();
  bool controlValuesChanged = true;

  // Inside temperature, measured locally
  if ( timestamp - waterTimestamp > MEASURE_INTERVAL )
  {
    waterTimestamp = timestamp;
    watertemp.requestTemperatures();
    int temp = watertemp.getTempCByIndex(0) * 10;
    if ( temp != waterTemperature )
    {
      waterTemperature = temp;
      controlValuesChanged = true;
      DEBUGONLY(Serial.print("Water temp changed to "));
      DEBUGONLY(Serial.println(waterTemperature));
    }
  }

  if ( controlValuesChanged )
  {
    
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
