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

#define SENDER_PIN   2
#define RECEIVER_PIN 5
#define DS18B20_PIN  4

const int INVALID_TEMP =       -1000;
#define   TEMP_VALIDITY       300000
#define   MEASURE_INTERVAL     10000
#define   FORCE_TIME_DURATION 300000

// Temp Display
#define DISPLAY_ADDRESS 0x3C
#define SDA_PIN 14
#define SCL_PIN 12
#include <SSD1306Wire.h>
#include "Open_Sans_Condensed_Bold_40.h"
SSD1306Wire  display(DISPLAY_ADDRESS, SDA_PIN, SCL_PIN);

// The values we preserve
bool          wantedPumpStatus = false;
InterUnitCommunication  pumpFeedback(0);
unsigned long pumpFeedbackTimestamp = 0;
unsigned long pumpReceived = 0;
unsigned long pumpReceivedTimestamp = 0;
int           pumpForceTime = 0;  // End time in ms
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
  pumpFeedback.temperature = INVALID_TEMP;
  rcv.start();
  insidetemp.begin();
  insidetemp.setResolution(12);
  // Temp Display
  display.init();
  display.flipScreenVertically();
}

// Format a temperature in 10ths of degrees to a nice string.
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
  bool displayChanged = true;
  bool controlValuesChanged = true;

  // outside temperature, received by RF433
  if ( outsideTemperature != INVALID_TEMP && timestamp - outsideTimestamp > TEMP_VALIDITY)
  {
    outsideTemperature = INVALID_TEMP;
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
      controlValuesChanged = true;
      DEBUGONLY(Serial.print("Inside temp changed to "));
      DEBUGONLY(Serial.println(insideTemperature));
    }
  }

  if ( controlValuesChanged )
  {
    
  }

  if ( displayChanged )
  {
    // Temp Display
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(Open_Sans_Condensed_Bold_40);
    //display.setFont(ArialMT_Plain_24);
    String str;
    ConcatTemp(insideTemperature, str);
    display.drawString(0, 0, str);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    str = "";
    ConcatTemp(outsideTemperature, str);
    display.drawString(128, 0, str);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 48, "CV: 45");
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128, 48, "P: aan?");
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
