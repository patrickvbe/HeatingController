#include <OneWire.h>
#include <DallasTemperature.h>
#include "src/InterUnitCommunication/InterUnitCommunication.h"

#define DEBUG
#ifdef DEBUG
#define DEBUGONLY(statement) statement;
#else
#define DEBUGONLY(statement)
#endif

#define PUMP_PIN 3
#define DS18B20_PIN 9
#define LED_BUILDIN 13

const int INVALID_TEMP = -1000;

#ifdef DEBUG
#define MINIMUM_COMMUNICATION_INTERVAL  15000UL
#define MASTER_VALIDITY                110000UL
#define MEASURE_INTERVAL                15000UL
#define FORCE_TIME_DURATION             30000UL
#define MAX_OFF_PERIOD                 120000UL
#else
#define MINIMUM_COMMUNICATION_INTERVAL  90000UL
#define MASTER_VALIDITY                300000UL
#define MEASURE_INTERVAL                10000UL
#define FORCE_TIME_DURATION            300000UL
#define MAX_OFF_PERIOD          1000UL*60*60*24
#endif

// The values we preserve
unsigned long lastValidMasterReceivedTimestamp = 0;  // Timestamp of the last valid master command
unsigned long masterReceived = 0;               // Last code received from the master
unsigned long masterSendTimestamp = 0;          // Timestamp we send the last data to the master
bool isPumpOn = false;                          // The pump status (on/off)
bool isForcedOn = false;                        // Is the pump forced-on (at least once per 24 hour running check)
bool inFallbackMode = false;                    // Fallback mode after communication timeout with the server
bool updateMaster = false;                      // Do we still need to update the master with a state change?
unsigned long pumpOffTimestamp = 0;             // The timestamp the pump was last turned of
int waterTemperatureSetpoint = 200;             // Temperature setpoint to turn the pump on / off in fallback mode
int waterTemperature = INVALID_TEMP;            // The last read valid temperature (invalidated after MASTER_VALIDITY ms)
unsigned long waterTimestamp = 0;               // The timestamp the last water temperature measurement was done.
unsigned long ForcedOnTimestamp = 0;            // The timestamp the pump is set on forced.
unsigned long start_response_delay = 0;         // The start of the period we will not send a response.
unsigned long timestamp = 0;                    // Global timestamp to freeze the time during the loop.

// The objects / sensors we have
InterUnitCommunication  communicator;  // For now, uses the default serial port.
OneWire                 onewire(DS18B20_PIN);
DallasTemperature       watertemp(&onewire);

void setup()
{
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH); // Turn the pump off initially (the relais is inverse controlled).
  pinMode(LED_BUILDIN, OUTPUT);
  digitalWrite(LED_BUILDIN, LOW); // Indicates fall-back mode.
  Serial.begin(115200);
  Serial.setTimeout(100);
  // Set some timestamps to the current millis to prevent fallback / forced mode immediately.
  timestamp = millis();
  pumpOffTimestamp = timestamp;
  watertemp.begin();
  watertemp.setResolution(10);
  // Used to enter back for debugging.
  // <200,1,0,225>
  // <200,0,0,37>
   DEBUGONLY(communicator.Send(200, true, false));
   DEBUGONLY(communicator.Send(200, false, false));
}

#ifdef DEBUG
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
#endif

void TurnPumpOn()
{
  isPumpOn = true;
  digitalWrite(PUMP_PIN, LOW);
  updateMaster = true;
}

void TurnPumpOff()
{
  isPumpOn = false;
  digitalWrite(PUMP_PIN, HIGH);
  pumpOffTimestamp = timestamp;
  updateMaster = true;
}

void loop()
{
  timestamp = millis(); // Freeze the time.

  // Water temperature, measured locally
  if ( (timestamp - waterTimestamp) > MEASURE_INTERVAL)
  {
    waterTimestamp = timestamp;
    watertemp.requestTemperatures();
    int temp = watertemp.getTempCByIndex(0) * 10;
    if (temp != waterTemperature)
    {
      waterTemperature = temp;
      updateMaster = true;
      DEBUGONLY(LogTime());
      DEBUGONLY(Serial.print(F("Water temp changed to ")));
      DEBUGONLY(Serial.println(waterTemperature));
    }
  }

  // Read data from master
  if ( communicator.Read() )
  {
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.print(F("Received valid master communication: ")));
    DEBUGONLY(Serial.print(communicator.m_temperature));
    DEBUGONLY(Serial.print(" "));
    DEBUGONLY(Serial.println(communicator.m_pumpOn));
    inFallbackMode = false;
    digitalWrite(LED_BUILDIN, LOW);
    lastValidMasterReceivedTimestamp = timestamp;
    // temperature: the setpoint for the pump to start
    // pumpOn:      the requested pump status
    // The others are ignored.
    waterTemperatureSetpoint = communicator.m_temperature;
    if (communicator.m_pumpOn )
    {
      isForcedOn = false;
      if ( !isPumpOn ) TurnPumpOn();
    }
    else
    {
      if ( isPumpOn && !isForcedOn ) TurnPumpOff();
    }
  }

  // Force pump on if already off for the maximum period.
  if ( !isPumpOn && (timestamp - pumpOffTimestamp) > MAX_OFF_PERIOD )
  {
    isForcedOn = true;
    ForcedOnTimestamp = timestamp;
    TurnPumpOn();
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.println(F("Turned pump on after long idle period")));
  }

  // Also, turn it off again in forced mode after a set duration.
  if ( isForcedOn && (timestamp - ForcedOnTimestamp) > FORCE_TIME_DURATION )
  {
    isForcedOn = false;
    TurnPumpOff();
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.println(F("Turned pump off after forced on")));
  }

  // If we don't receive anything from our master for a long time, go in fallback mode.
  if ( !inFallbackMode && (timestamp - lastValidMasterReceivedTimestamp) > MASTER_VALIDITY )
  {
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.println(F("Switched to fallback mode")));
    inFallbackMode = true;
    digitalWrite(LED_BUILDIN, HIGH);
  }

  // Run in fallback mode.
  if ( inFallbackMode && !isForcedOn )
  {
    if ( isPumpOn )
    {
      if ( waterTemperature < waterTemperatureSetpoint )
      {
        TurnPumpOff();
        DEBUGONLY(Serial.println(F("Switched off in fallback mode")));
      }
    }
    else
    {
      if ( waterTemperature >= (waterTemperatureSetpoint + 10) )
      {
        TurnPumpOn();
        DEBUGONLY(Serial.println(F("Switched on in fallback mode")));
      }
    }
  }

  // Update the master at least every MINIMUM_COMMUNICATION_INTERVAL ms.
  if ( !updateMaster && timestamp - masterSendTimestamp > MINIMUM_COMMUNICATION_INTERVAL )
  {
    updateMaster = true;
  }

  // If needed, communicate our status to the master.
  if ( updateMaster )
  {
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.println(F("Send update to our master.")));
    communicator.Send(waterTemperature > 0 ? waterTemperature : 0, isPumpOn, isForcedOn);
    masterSendTimestamp = timestamp;
    updateMaster = false;    
  }

} // loop()
