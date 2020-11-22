//////////////////////////////////////////////////////////////
// Global control values
//////////////////////////////////////////////////////////////

#ifndef CONTROLVALUES_H
#define CONTROLVALUES_H

#define DEFAULT_INSIDE_TEMP_SETPOINT    200
#define INVALID_TEMP -1000
#define INVALID_TIME -1000000

// All times are in ms.
class ControlValues
{
  public:

    unsigned long LastValidPumpTimestamp = 0;                   // The last time we received valid information from the pump unit.
    unsigned long pumpSendTimestamp = INVALID_TIME;             // Timestamp of last information send to the pump unit.
    int           waterTemperature  = INVALID_TEMP;             // CV water temperature received from the pump unit.
    bool          isPumpOn = false;                             // Pump status received from the pump unit.
    bool          isPumpForced = false;                         // Forced status received from the pump unit.
    bool          pumpNeedsOn = false;                          // Our computed / wanted pump status.
    bool          pumpCommunicationOK = false;                  // Did we receive valid and on-time communication from the pump unit?
    int           outsideTemperature = INVALID_TEMP;            // New temperature received from the weather station.
    int           insideTemperature  = INVALID_TEMP;            // Last measured inside temperature.
    int           waterTemperatureSetpoint = 220;               // CV water temperature setpoint to turn the pump on / off. Might be configurable in the future.
    int           insideTemperatureSetpoint = DEFAULT_INSIDE_TEMP_SETPOINT; // Room temperature setpoint to turn the pump on / off. Might be configurable in the future.
    int           displaymode = 0;                              // What to show on the screen
    unsigned long insideSetpointDuration = 0;                   // The validity of the modified inside setpoint. 0 = don't change back.
    unsigned long insideSetpointStart=0;                        // When did the inside setpoint changed?
    unsigned long outsideTimestamp  = 0;                        // Timestamp of the last valid outside temperature received.
    unsigned long insideTimestamp = INVALID_TIME;               // Timestamp the inside temperature was last measured.
    unsigned long lastforcedon=0;
    bool          insideRequested = false;
    char          wifiStatus = '-'; // '-' not connected, '+' connected, '#' got IP.
};

#endif // CONTROLVALUES_H
