#include <OneWire.h>
#include <DallasTemperature.h>
#include "src/RF433/RF433.h"
#include "src/InterUnitCommunication/InterUnitCommunication.h"

//#define DEBUG
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
#define LED_BUILDIN 13

const int INVALID_TEMP = -1000;

// When we receive a signal from the master, the master is probably sending 1 to 10 signals.
// Each signal takes app. 300ms. When we catch the first signal, the master might still be
// busy for almost 3 seconds sending the repeats and thus not listening / using the frequency.
#define RESPONSE_DELAY 6000

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
unsigned long lastMasterReceivedTimestamp = 0;  // Timestamp of the last processed master command
unsigned long lastValidMasterReceivedTimestamp = 0;  // Timestamp of the last valid master command
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
unsigned long start_response_delay = 0;         // The start of the period we will not send a response.
unsigned long timestamp = 0;                    // Global timestamp to freeze the time during the loop.

// Interrupt routine for RF433 communication.
void code_received(int protocol, unsigned long code, unsigned long timestamp)
{
  if ( protocol == PUMP_CONTROLLER )
  {
    masterReceived = code;
    masterReceivedTimestamp = timestamp;
    //DEBUGONLY(LogTime());
    //DEBUGONLY(Serial.print(F("Received from master: ")));
    //DEBUGONLY(Serial.println(code, HEX));
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
DallasTemperature watertemp(&onewire);

void setup()
{
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH); // Turn the pump off initially (the relais is inverse controlled).
  pinMode(LED_BUILDIN, OUTPUT);
  digitalWrite(LED_BUILDIN, LOW); // Indicates fall-back mode.
  // Set some timestamps to the current millis to prevent fallback / forced mode immediately.
  timestamp = millis();
  pumpOffTimestamp = timestamp;
  masterReceivedTimestamp = timestamp;
  lastMasterReceivedTimestamp = timestamp;
  DEBUGONLY(Serial.begin(230400));
  rcv.start();
  watertemp.begin();
  watertemp.setResolution(10);
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

  // Process command received from the master
  if (masterReceivedTimestamp != lastMasterReceivedTimestamp)
  {
    if ( start_response_delay == 0 ) start_response_delay = masterReceivedTimestamp;
    lastMasterReceivedTimestamp = masterReceivedTimestamp;
    InterUnitCommunication received(masterReceived);
    if (received.isValid && received.unitCode == UNIT_CODE)
    {
      DEBUGONLY(LogTime());
      DEBUGONLY(Serial.print(F("Received valid master communication: ")));
      DEBUGONLY(Serial.print(received.temperature));
      DEBUGONLY(Serial.print(" "));
      DEBUGONLY(Serial.println(received.pumpOn));
      inFallbackMode = false;
      digitalWrite(LED_BUILDIN, LOW);
      lastValidMasterReceivedTimestamp = masterReceivedTimestamp;
      // temperature: the setpoint for the pump to start
      // pumpOn:      the requested pump status
      // The others are ignored.
      waterTemperatureSetpoint = received.temperature;
      if (received.pumpOn )
      {
        isForcedOn = false;
        if ( !isPumpOn ) TurnPumpOn();
      }
      else
      {
        if ( isPumpOn && !isForcedOn ) TurnPumpOff();
      }
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

  // See if we already passed the response delay.
  if ( start_response_delay != 0 && timestamp - start_response_delay > RESPONSE_DELAY ) start_response_delay = 0;

  // If needed, communicate our status to the master.
  if ( updateMaster && start_response_delay == 0 )
  {
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.print(F("Send update to our master: ")));
    DEBUGONLY(Serial.println(InterUnitCommunication::CalculateCode(UNIT_CODE, waterTemperature, isPumpOn, isForcedOn), HEX));
    rcv.stop();
    snd.send(PUMP_CONTROLLER, InterUnitCommunication::CalculateCode(UNIT_CODE, waterTemperature > 0 ? waterTemperature : 0, isPumpOn, isForcedOn), 6);
    rcv.start();
    masterSendTimestamp = timestamp;
    updateMaster = false;    
  }

#ifdef DEBUG
  // Test code
  int cin = Serial.read();
  if (cin == '1')
  {
    rcv.stop();
    snd.send(ELRO, 0x145151, 10);
    rcv.start();
    Serial.println(F("Send on"));
  }
  else if (cin == '0')
  {
    rcv.stop();
    snd.send(ELRO, 0x145154, 10);
    rcv.start();
    Serial.println(F("Send off"));
  }
  else if (cin == 'm')
  {
    rcv.stop();
    snd.send(PUMP_CONTROLLER, InterUnitCommunication::CalculateCode(UNIT_CODE, waterTemperature, isPumpOn, isForcedOn), 10);
    rcv.start();
    Serial.print(F("Send master: "));
    DEBUGONLY(Serial.println(InterUnitCommunication::CalculateCode(UNIT_CODE, waterTemperature, isPumpOn, isForcedOn), HEX));
  }
#endif
}
