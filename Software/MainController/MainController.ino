#include <OneWire.h>
#include <DallasTemperature.h>
#include "src/RF433/RF433.h"
#include "src/InterUnitCommunication/InterUnitCommunication.h"

#define DEBUG;
#ifdef DEBUG
  #define DEBUGONLY(statement) statement;
#else
  #define DEBUGONLY()
#endif

#define SENDER_PIN   2
#define RECEIVER_PIN 5
#define DS18B20_PIN  4
#define UNIT_CODE 0xBAF

const int INVALID_TEMP =       -1000;
#ifdef DEBUG
#define TEMP_VALIDITY                  300000
#define MINIMUM_COMMUNICATION_INTERVAL  20000
#define PUMP_VALIDITY                   30000
#define MEASURE_INTERVAL                10000
#define FORCE_TIME_DURATION             30000
#else
#define TEMP_VALIDITY                  300000
#define MINIMUM_COMMUNICATION_INTERVAL 240000
#define PUMP_VALIDITY                  300000
#define MEASURE_INTERVAL                10000
#define FORCE_TIME_DURATION            300000
#endif

// Temp Display
#define DISPLAY_ADDRESS 0x3C
#define SDA_PIN 14
#define SCL_PIN 12
#include <SSD1306Wire.h>
#include "Open_Sans_Condensed_Bold_40.h"
SSD1306Wire  display(DISPLAY_ADDRESS, SDA_PIN, SCL_PIN);

// The values we preserve
unsigned long pumpReceived = 0;
unsigned long pumpReceivedTimestamp = 0;
unsigned long pumpSendTimestamp = 0;
int           waterTemperature  = INVALID_TEMP;
bool          isPumpOn = false;
bool          isPumpForced = false;
bool          pumpNeedsOn = false;
bool          pumpCommunicationOK = false;
unsigned long LastPumpTimestamp = 0;
int           pumpForceTime = 0;  // End time in ms
int           outsideTemperature = INVALID_TEMP;
int           outsidePreviousTemperature = INVALID_TEMP;
unsigned long outsideTimestamp  = 0; // millis
int           insideTemperature  = INVALID_TEMP;
unsigned long insideTimestamp = 0;
int waterTemperatureSetpoint = 200;             // Temperature setpoint to turn the pump on / off. Might be configurable in the future.
int insideTemperatureSetpoint = 190;

// Interrupt routine for RF433 communication.
void code_received(int protocol, unsigned long code, unsigned long timestamp)
{
  if ( protocol == WEATHERSTATION)
  {
    outsideTemperature = receiver::convertCodeToTemp(code);
    outsideTimestamp = millis();
  }
  if ( protocol == PUMP_CONTROLLER)
  {
    pumpReceivedTimestamp = timestamp;
    pumpReceived = code;
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

void LogTime()
{
  unsigned long totalseconds = millis()/1000;
  unsigned long seconds = totalseconds % 60;
  unsigned long minutes = totalseconds / 60;
  Serial.print(minutes);
  Serial.print(F(":"));
  if ( seconds < 10 ) Serial.print("0");
  Serial.print(seconds);
  Serial.print(' ');
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
  bool controlValuesChanged = false;
  bool sendToPump = false;

  // outside temperature, received by RF433
  if ( outsideTemperature != INVALID_TEMP && timestamp - outsideTimestamp > TEMP_VALIDITY)
  {
    outsideTemperature = INVALID_TEMP;
  }
  if ( outsidePreviousTemperature != outsideTemperature )
  {
    outsidePreviousTemperature = outsideTemperature;
    displayChanged = true;
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.print("Outside temp changed to "));
    DEBUGONLY(Serial.println(outsideTemperature));
  }

  // Communication from pump controller
  if ( pumpReceivedTimestamp != LastPumpTimestamp )
  {
    InterUnitCommunication pumpcomm(pumpReceived);
    if ( pumpcomm.isValid && pumpcomm.unitCode == UNIT_CODE )
    {
      LastPumpTimestamp = pumpReceivedTimestamp;
      pumpCommunicationOK = true;
      if ( waterTemperature != pumpcomm.temperature || isPumpOn != pumpcomm.pumpOn )
      {
        controlValuesChanged = true;
        displayChanged = true;
        waterTemperature = pumpcomm.temperature;
        isPumpOn = pumpcomm.pumpOn;
      }
      if ( isPumpForced != pumpcomm.pumpForcedOn )
      {
        isPumpForced = pumpcomm.pumpForcedOn;
        displayChanged = true;
      }
      DEBUGONLY(LogTime());
      DEBUGONLY(Serial.println("Received update from pump controller"));
    }

  }

  if ( timestamp - LastPumpTimestamp > PUMP_VALIDITY )
  {
    pumpCommunicationOK = false;
    waterTemperature = INVALID_TEMP;
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
      DEBUGONLY(LogTime());
      DEBUGONLY(Serial.print("Inside temp changed to "));
      DEBUGONLY(Serial.println(insideTemperature));
    }
  }

  // The actual controlling actions.
  if ( controlValuesChanged )
  {
    if ( waterTemperature != INVALID_TEMP && insideTemperature != INVALID_TEMP )
    {
      if ( pumpNeedsOn )
      {
        if ( insideTemperature >= insideTemperatureSetpoint + 10 || waterTemperature <= waterTemperatureSetpoint )
        {
          pumpNeedsOn = false;
          sendToPump = true;
        }
      }
      else
      {
        if ( insideTemperature <= insideTemperatureSetpoint && waterTemperature >= waterTemperatureSetpoint + 10 )
        {
          pumpNeedsOn = true;
          sendToPump = true;
        }
      }      
    }
  }

  // Update the pump at least every MINIMUM_COMMUNICATION_INTERVAL ms.
  if ( !sendToPump && timestamp - pumpSendTimestamp > MINIMUM_COMMUNICATION_INTERVAL )
  {
    sendToPump = true;
  }

  // If needed, communicate our status to the master.
  if ( sendToPump )
  {
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.println(F("Send update to pump.")));
    snd.send(PUMP_CONTROLLER, InterUnitCommunication::CalculateCode(UNIT_CODE, waterTemperatureSetpoint, pumpNeedsOn, isPumpForced /* ignored by pump */), 2);
    pumpSendTimestamp = timestamp;
    sendToPump = false;    
  }

  // Update the display for the user.
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
    if ( waterTemperature != INVALID_TEMP )
    {
      str = "CV: ";
      str.concat(waterTemperature);
    }
    else
    {
      str = "CV: --";
    }
    display.drawString(0, 46, str);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    if ( pumpCommunicationOK )
    {
      str = "P: ";
      if ( isPumpOn )
      {
        if ( isPumpForced ) str.concat("AAN");
        else str.concat("aan");
      }
      else str.concat("uit");
      if ( !pumpNeedsOn ) str.concat("!");
    }
    else
    {
      str = "p-fout";
    }
    display.drawString(128, 46, str);
    
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
