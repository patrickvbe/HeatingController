#include "RF433.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define DEBUG;
#ifdef DEBUG
  #define DEBUGONLY(statement) statement;
#else
  #define DEBUGONLY()
#endif

#define SENDER_PIN   2
#define RECEIVER_PIN 5
#define DS18B20_PIN  4

#define INVALID_TEMP      -1000
#define TEMP_VALIDITY     300000
#define MEASURE_INTERVAL   10000

// Temp Display
#define DISPLAY_ADDRESS 0x3C
#define SDA_PIN 14
#define SCL_PIN 12
#include <SSD1306Wire.h>
SSD1306Wire  display(DISPLAY_ADDRESS, SDA_PIN, SCL_PIN);

// The values we preserve
bool          displayChanged = true;
bool          controlValuesChanged = true;
int           outsideTemperature = INVALID_TEMP;
int           outsidePreviousTemperature = INVALID_TEMP;
unsigned long outsideTimestamp  = 0; // millis
int           insideTemperature  = INVALID_TEMP;
unsigned long insideTimestamp = 0;

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
OneWire           onewire(DS18B20_PIN);
DallasTemperature insidetemp(&onewire);

void setup()
{
  Serial.begin(230400);
  rcv.start();
  insidetemp.begin();
  insidetemp.setResolution(12);
  // Temp Display
  display.init();
  display.flipScreenVertically();
}

void ConcatTemp(int temp, String& str)
{
  if ( temp != INVALID_TEMP )
  {
    str.concat(temp / 10);
    str.concat(".");
    str.concat(temp % 10);
  } 
  else
  {
    str.concat("--.-");
  }
  
}

void loop()
{
  unsigned long timestamp = millis();

  // outside temperature, received by RF433
  if ( outsideTemperature != INVALID_TEMP && timestamp - outsideTimestamp > TEMP_VALIDITY)
  {
    outsideTemperature = INVALID_TEMP;
    displayChanged = true;
  }
  if ( outsidePreviousTemperature != outsideTemperature )
  {
    outsidePreviousTemperature = outsideTemperature;
    displayChanged = true;
    DEBUGONLY(Serial.print("Outside temp changed to "));
    DEBUGONLY(Serial.println(outsideTemperature));
  }

  // Inside temperature, measured locally
  if ( timestamp - insideTimestamp > MEASURE_INTERVAL )
  {
    insideTimestamp = timestamp;
    insidetemp.requestTemperatures();
    int temp = insidetemp.getTempCByIndex(0) * 10;
    if ( temp != insideTemperature )
    {
      insideTemperature = temp;
      displayChanged = true;
      DEBUGONLY(Serial.print("Inside temp changed to "));
      DEBUGONLY(Serial.println(insideTemperature));
    }
  }

  if ( displayChanged )
  {
    // Temp Display
    display.clear();
    display.setFont(ArialMT_Plain_24);
    String str;
    ConcatTemp(insideTemperature, str);
    display.drawString(0, 0, str);
    str = "Buiten: ";
    ConcatTemp(outsideTemperature, str);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 24, str);
    display.display();
    displayChanged = false;
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
