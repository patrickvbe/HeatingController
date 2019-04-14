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
unsigned long pumpReceived = 0;                             // Raw code received from the pump unit.
unsigned long pumpReceivedTimestamp = 0;                    // Timestamp of reception from the pump unit.
unsigned long LastPumpTimestamp = 0;                        // The last time we received information from the pump unit.
unsigned long LastValidPumpTimestamp = 0;                   // The last time we received valid information from the pump unit.
unsigned long pumpSendTimestamp = 0;                        // Timestamp of last information send to the pump unit.
int           waterTemperature  = INVALID_TEMP;             // CV water temperature received from the pump unit.
bool          isPumpOn = false;                             // Pump status received from the pump unit.
bool          isPumpForced = false;                         // Forced status received from the pump unit.
bool          pumpNeedsOn = false;                          // Our computed / wanted pump status.
bool          pumpCommunicationOK = false;                  // Did we receive valid and on-time communication from the pump unit?
int           outsideTemperature = INVALID_TEMP;            // New temperature received from the weather station.
int           outsidePreviousTemperature = INVALID_TEMP;    // Previous outside temperature (now displayed)
unsigned long outsideTimestamp  = 0;                        // Timestamp of the last valid outside temperature received.
unsigned long insideTimestamp = 0;                          // Timestamp the inside temperature was last measured.
int           insideTemperature  = INVALID_TEMP;            // Last measured inside temperature.
int           waterTemperatureSetpoint = 200;               // CV water temperature setpoint to turn the pump on / off. Might be configurable in the future.
int           insideTemperatureSetpoint = 190;              // Room temperature setpoint to turn the pump on / off. Might be configurable in the future.

// Interrupt routine for RF433 communication.
void code_received(int protocol, unsigned long code, unsigned long timestamp)
{
  if ( protocol == WEATHERSTATION)
  {
    outsideTemperature = receiver::convertCodeToTemp(code);
    outsideTimestamp = timestamp;
    Serial.print("Outside: ");
    Serial.println(outsideTemperature);
  }
  if ( protocol == PUMP_CONTROLLER)
  {
    pumpReceivedTimestamp = timestamp;
    pumpReceived = code;
  }
   Serial.print("Protocol: ");
   Serial.print(protocol);
   Serial.print(", code:");
   Serial.print(code, BIN);
   Serial.print(" / ");
   Serial.print(code, HEX);
   Serial.println();
}

// The objects / sensors we have
receiver          rcv(RECEIVER_PIN, code_received);
sender            snd(SENDER_PIN);
OneWire           onewire(DS18B20_PIN);
DallasTemperature insidetemp(&onewire);

void setup()
{
  Serial.begin(115200);
  rcv.start();
  insidetemp.begin();
  insidetemp.setResolution(12);
  // Display
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
  unsigned long timestamp = millis();   // Freeze the time
  bool displayChanged = true;           // We only want to update the display if displayed values changed.
  bool controlValuesChanged = false;    // We only want to calculate the logic when input values changed (a bit over-the-top...)
  bool sendToPump = false;              // Immediately communicate if values change / differ, otherwise in a timed interval.

  // outside temperature, received by RF433
  if ( outsideTemperature != INVALID_TEMP && timestamp - outsideTimestamp > TEMP_VALIDITY)
  {
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.print("Invalidated outside T from "));
    DEBUGONLY(Serial.print(outsideTemperature));
    DEBUGONLY(Serial.print(" "));
    DEBUGONLY(Serial.print(timestamp));
    DEBUGONLY(Serial.print(" "));
    DEBUGONLY(Serial.println(outsideTimestamp));
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
    LastPumpTimestamp = pumpReceivedTimestamp;
    InterUnitCommunication pumpcomm(pumpReceived);
    if ( pumpcomm.isValid && pumpcomm.unitCode == UNIT_CODE )
    {
      LastValidPumpTimestamp = pumpReceivedTimestamp;
      pumpCommunicationOK = true;
      if ( waterTemperature != pumpcomm.temperature )
      {
        controlValuesChanged = true;
        displayChanged = true;
        waterTemperature = pumpcomm.temperature;
      }
      if ( isPumpForced != pumpcomm.pumpForcedOn )
      {
        isPumpForced = pumpcomm.pumpForcedOn;
        displayChanged = true;
      }
      if ( isPumpOn != pumpcomm.pumpOn )
      {
        displayChanged = true;
        isPumpOn = pumpcomm.pumpOn;
        if (isPumpOn != pumpNeedsOn && !isPumpForced) sendToPump = true;
      }
      DEBUGONLY(LogTime());
      DEBUGONLY(Serial.println("Received update from pump controller"));
    }
    else
    {
      DEBUGONLY(LogTime());
      DEBUGONLY(Serial.println("Invalid pump message"));
    }

  }

  if ( pumpCommunicationOK && timestamp - LastValidPumpTimestamp > PUMP_VALIDITY )
  {
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.print("Invalidated pump communication "));
    DEBUGONLY(Serial.print(timestamp));
    DEBUGONLY(Serial.print(" "));
    DEBUGONLY(Serial.println(LastValidPumpTimestamp));
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
        if ( insideTemperature >= insideTemperatureSetpoint + 5 || waterTemperature <= waterTemperatureSetpoint )
        {
          pumpNeedsOn = false;
          displayChanged = true;
          sendToPump = true;
        }
      }
      else
      {
        if ( insideTemperature <= insideTemperatureSetpoint && waterTemperature >= waterTemperatureSetpoint + 10 )
        {
          pumpNeedsOn = true;
          displayChanged = true;
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
    rcv.stop();
    snd.send(PUMP_CONTROLLER, InterUnitCommunication::CalculateCode(UNIT_CODE, waterTemperatureSetpoint, pumpNeedsOn, isPumpForced /* ignored by pump */), 6);
    rcv.start();
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
      str.concat(waterTemperature / 10);
    }
    else
    {
      str = "CV: --";
    }
    display.drawString(0, 46, str);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    if ( pumpCommunicationOK )
    {
      str = "P ";
      if ( isPumpForced )
      {
        str.concat("AAN");
      }
      else
      {
        str.concat(isPumpOn ? "aan" : "uit");
        if ( pumpNeedsOn != isPumpOn ) str.concat("!");
      }
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
  else if (cin == 'p')
  {
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.println(F("Force to pump.")));
    rcv.stop();
    snd.send(PUMP_CONTROLLER, InterUnitCommunication::CalculateCode(UNIT_CODE, waterTemperatureSetpoint, pumpNeedsOn, isPumpForced /* ignored by pump */), 10);
    rcv.start();
  }
}
