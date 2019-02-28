#include <OneWire.h>
#include <DallasTemperature.h>
#include "src/RF433/RF433.h"
#include "src/InterUnitCommunication/InterUnitCommunication.h"

#define DEBUG ;
#ifdef DEBUG
#define DEBUGONLY(statement) statement;
#else
#define DEBUGONLY(statement)
#endif

#define PUMP_PIN 3
#define SENDER_PIN 5
#define RECEIVER_PIN 2
#define DS18B20_PIN 9
#define UNIT_CODE 0xBAF

const int INVALID_TEMP = -1000;

#ifdef DEBUG
#define MINIMUM_COMMUNICATION_INTERVAL  20000
#define MASTER_VALIDITY                 30000
#define MEASURE_INTERVAL                 4000
#define FORCE_TIME_DURATION             30000
#define MAX_OFF_PERIOD                 120000
#else
#define MINIMUM_COMMUNICATION_INTERVAL 240000
#define MASTER_VALIDITY                300000
#define MEASURE_INTERVAL                10000
#define FORCE_TIME_DURATION            300000
#define MAX_OFF_PERIOD          1000*60*60*24
#endif

// The values we preserve
unsigned long lastMasterReceivedTimestamp = 0;  // Timestamp of the last processed master command
unsigned long masterReceived = 0;               // Last code received from the master
unsigned long masterReceivedTimestamp = 0;      // Timestamp of the last code received from the master
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

// Interrupt routine for RF433 communication.
void code_received(int protocol, unsigned long code, unsigned long timestamp)
{
  if ( protocol == PUMP_CONTROLLER )
  {
    masterReceived = code;
    masterReceivedTimestamp = timestamp;
  }
}

// The objects / sensors we have
receiver          rcv(RECEIVER_PIN, code_received);
sender            snd(SENDER_PIN);
OneWire           onewire(DS18B20_PIN);
DallasTemperature watertemp(&onewire);

void setup()
{
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH); // Turn the pump off initially (the relais is inverse controlled).
  // Set some timestamps to the current millis to prevent fallback / forced mode immediately.
  pumpOffTimestamp = millis();
  masterReceivedTimestamp = pumpOffTimestamp;
  lastMasterReceivedTimestamp = pumpOffTimestamp;
  Serial.begin(230400);
  rcv.start();
  watertemp.begin();
  watertemp.setResolution(10);
}

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
  pumpOffTimestamp = millis();
  updateMaster = true;
}

void loop()
{
  unsigned long timestamp = millis(); // Freeze the time.

  // Water temperature, measured locally
  if (timestamp - waterTimestamp > MEASURE_INTERVAL)
  {
    waterTimestamp = timestamp;
    watertemp.requestTemperatures();
    int temp = watertemp.getTempCByIndex(0) * 10;
    if (temp != waterTemperature)
    {
      waterTemperature = temp;
      updateMaster = true;
      DEBUGONLY(Serial.print(F("Water temp changed to ")));
      DEBUGONLY(Serial.println(waterTemperature));
    }
  }

  // Process command received from the master
  if (masterReceivedTimestamp != lastMasterReceivedTimestamp)
  {
    InterUnitCommunication received(masterReceived);
    if (received.isValid && received.unitCode == UNIT_CODE)
    {
      inFallbackMode = false;
      lastMasterReceivedTimestamp = masterReceivedTimestamp;
      // temperature: the setpoint for the pump to start
      // pumpOn:      the requested pump status
      // The others are ignored.
      DEBUGONLY(Serial.print(F("Received valid master communication: ")));
      DEBUGONLY(Serial.print(waterTemperatureSetpoint));
      DEBUGONLY(Serial.print(" "));
      DEBUGONLY(Serial.println(received.pumpOn));
      waterTemperatureSetpoint = received.temperature;
      if (received.pumpOn != isPumpOn && !isForcedOn )
      {
        if ( isPumpOn )
        {
          TurnPumpOff();
        }
        else
        {
          TurnPumpOn();
        }
      }
    }
  }

  // Force pump on if already off for the maximum period.
  if ( !isPumpOn && timestamp - pumpOffTimestamp > MAX_OFF_PERIOD )
  {
    isForcedOn = true;
    ForcedOnTimestamp = timestamp;
    TurnPumpOn();
    DEBUGONLY(Serial.println(F("Turned pump on after long idle period")));
  }

  // Also, turn it off again in forced mode after a set duration.
  if ( isForcedOn && timestamp - ForcedOnTimestamp > FORCE_TIME_DURATION )
  {
    isForcedOn = false;
    TurnPumpOff();
    DEBUGONLY(Serial.println(F("Turned pump of after forced on")));
  }

  // If we don't receive anything from our master for a long time, go in fallback mode.
  if ( !inFallbackMode && timestamp - lastMasterReceivedTimestamp > MASTER_VALIDITY )
  {
    DEBUGONLY(Serial.println(F("Switched to fallback mode")));
    inFallbackMode = true;
  }

  // Run in fallback mode.
  if ( inFallbackMode && !isForcedOn )
  {
    if ( isPumpOn )
    {
      if ( waterTemperature < waterTemperatureSetpoint )
      {
        TurnPumpOff();
      }
    }
    else
    {
      if ( waterTemperature >= waterTemperatureSetpoint + 10)
      {
        TurnPumpOff();
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
    snd.send(PUMP_CONTROLLER, InterUnitCommunication::CalculateCode(UNIT_CODE, waterTemperature, isPumpOn, isForcedOn));
    masterSendTimestamp = timestamp;
    updateMaster = false;    
    DEBUGONLY(Serial.println(F("Send update to our master.")));
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
