// python3 ~/.arduino15/packages/esp8266/hardware/esp8266/2.7.4/tools/espota.py -i 192.168.178.115 -p 8266 -f /tmp/arduino_build_661492/MainController.ino.bin
// LOLIN D1 mini
// ESP8266 and ESP32 driver for SSD1306 display, ThingPulse, Fabrice Weinberg. Version 4.0.0
// Going OTA...
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "network_secrets.h"
//#define STASSID "your-ssid"
//#define STAPSK  "your-password"
const char* ssid = STASSID;
const char* password = STAPSK;
bool doingota = false;

// Sensors, etc.
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

//////////////////////////////////////////////////////////////
// Pin settings
//////////////////////////////////////////////////////////////
// IO0 = D3, has pull-up resistor
// IO1 = TXD0 = RX
// IO2 = D4, has pull-up resistor
// IO3 = RXD0 = TX
// IO4 = D2
// IO5 = D1
// IO12 = D6
// IO13 = D7
// IO14 = D5

// RF433:
#define SENDER_PIN    5
#define RECEIVER_PIN  4
// Inside temperature
#define DS18B20_PIN  13
#define DS18B20_DELAY 1000
// Buttons
#define BUTTON1_PIN 0
#define BUTTON2_PIN 2
// Display
#define DISPLAY_ADDRESS 0x3C
#define SDA_PIN 12
#define SCL_PIN 14

//////////////////////////////////////////////////////////////
// Control constants
//////////////////////////////////////////////////////////////
#ifdef DEBUG
#define TEMP_VALIDITY                  900000
#define MINIMUM_COMMUNICATION_INTERVAL  25000
#define PUMP_VALIDITY                   70000
#define MEASURE_INTERVAL                10000
#define FORCE_TIME_DURATION             30000
#else
#define TEMP_VALIDITY                  900000
#define MINIMUM_COMMUNICATION_INTERVAL  60000
#define PUMP_VALIDITY                  300000
#define MEASURE_INTERVAL                15000
#define FORCE_TIME_DURATION            300000
#endif

// Dynamic control values
#include "ControlValues.h"
ControlValues ctrl;
static unsigned long lastloopmillis;

// WiFi
WiFiEventHandler mConnectHandler, mDisConnectHandler, mGotIpHandler;
const unsigned long WIFI_TRY_INTERVAL = 60000;
unsigned long lastWiFiTry = -WIFI_TRY_INTERVAL;

// Communication OpenTherm gateway
const unsigned long OTOUTSIDE_INTERVAL = 300000;        // Read at least every 5 minutes.
const unsigned long OTOUTSIDE_RETRY = 30000;            // Retry every 30 seconds when we fail.
const unsigned long OTOUTSIDE_INVALID_TIMEOUT = 600000; // When failing for 10 minutes, declare the temp invalid.
bool otRequested = false; // Was a request send to get the OT outside temp?
unsigned long lastOTOutsideTemp = millis() - OTOUTSIDE_INTERVAL;
unsigned long lastOTOutsideTempOK = millis() - OTOUTSIDE_INVALID_TIMEOUT;

//////////////////////////////////////////////////////////////
// Include display control
//////////////////////////////////////////////////////////////
#ifdef DEBUG
  #include <SSD1306Wire.h>
  SSD1306Wire  display(DISPLAY_ADDRESS, SDA_PIN, SCL_PIN);
#else
  #include <SH1106Wire.h>
  SH1106Wire  display(DISPLAY_ADDRESS, SDA_PIN, SCL_PIN);
#endif

#include "Screen.h"
Screen screen(display, ctrl, BUTTON1_PIN, BUTTON2_PIN);

#include "WebServer.h"

//////////////////////////////////////////////////////////////
// The objects / sensors we have
//////////////////////////////////////////////////////////////
receiver                rcv(RECEIVER_PIN);
sender                  snd(SENDER_PIN);
InterUnitCommunication  communicator;  // For now, uses the default serial port.
OneWire                 onewire(DS18B20_PIN);
DallasTemperature       insidetemp(&onewire);

/***************************************************************
 * Show a message full-screen in a big font.
 ***************************************************************/
// void ShowFullScreenStatus(char* status)
// {
//   display.clear();
//   display.setTextAlignment(TEXT_ALIGN_LEFT);
//   display.setFont(Open_Sans_Condensed_Bold_30);
//   display.drawString(0, 0, status);
//   display.display();
// }

/***************************************************************
 * The global initialization function.
 ***************************************************************/
void setup()
{
  Serial.begin(115200);

  //////////////////////////////////////////////////////////////
  // Display setup
  //////////////////////////////////////////////////////////////
  display.init();
  display.flipScreenVertically();

  //////////////////////////////////////////////////////////////
  // WiFi setup
  //////////////////////////////////////////////////////////////
  WiFi.mode(WIFI_STA);
  WiFi.disconnect() ;
  WiFi.persistent(false);
  mDisConnectHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected&)
  {
    ctrl.wifiStatus = '-';
    screen.TriggerUpdate(); 
  });
  mConnectHandler = WiFi.onStationModeConnected([](const WiFiEventStationModeConnected&)
  {
    ctrl.wifiStatus = '+';
    screen.TriggerUpdate(); 
  });
  mGotIpHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP&)
  {
    ctrl.wifiStatus = '#';
    screen.TriggerUpdate(); 
    ArduinoOTA.begin();
    MDNS.begin("Thermostat");
  });

  //////////////////////////////////////////////////////////////
  // OTA setup
  //////////////////////////////////////////////////////////////
#ifdef DEBUG
  ArduinoOTA.setHostname("DevHeat");
#else
  ArduinoOTA.setHostname("HeatController");
#endif
  ArduinoOTA.onStart([]() {
    doingota = true;
    rcv.stop();
    //ShowFullScreenStatus("OTA");
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
  });
  ArduinoOTA.onEnd([]() {
    rcv.start();
    doingota = false;
  });
  ArduinoOTA.onError([](ota_error_t error) {
    //ShowFullScreenStatus("ota failed");
    delay(1000);
    rcv.start();
    doingota = false;
  });

  //////////////////////////////////////////////////////////////
  // 
  //////////////////////////////////////////////////////////////
  webserver.Init(ctrl);
  rcv.start();
  insidetemp.begin();
  insidetemp.setResolution(12);
  insidetemp.setWaitForConversion(false);
  lastloopmillis = millis();
}

/***************************************************************
 * Log time since startup in a nicely readable format.
 ***************************************************************/
#ifdef DEBUG
void LogTime()
{
  unsigned long totalseconds = millis()/1000;
  unsigned long seconds = totalseconds % 60;
  unsigned long minutes = totalseconds / 60;
  Serial.print(minutes);
  Serial.print(':');
  if ( seconds < 10 ) Serial.print('0');
  Serial.print(seconds);
  Serial.print(' ');
}
#endif

/***************************************************************
 * Check if the control values are valid
 ***************************************************************/
boolean ControlValuesAreValid()
{
  return ctrl.waterTemperature != INVALID_TEMP && ctrl.insideTemperature != INVALID_TEMP;
}

/***************************************************************
 * Main loop
 ***************************************************************/
void loop()
{
  //////////////////////////////////////////////////////////////
  // OTA
  //////////////////////////////////////////////////////////////
  ArduinoOTA.handle();
  if ( doingota ) return;

  webserver.Process();
  MDNS.update();

  bool controlValuesChanged = false;    // We only want to calculate the logic when input values changed (a bit over-the-top...)
  bool sendToPump = false;              // Do we need to update the pump controller?
  auto timestamp = millis(); // Freeze the time

  //////////////////////////////////////////////////////////////
  // Try to stay connected to the WiFi.
  //////////////////////////////////////////////////////////////
  if (WiFi.status() != WL_CONNECTED && ctrl.wifiStatus != '#' && timestamp - lastWiFiTry > WIFI_TRY_INTERVAL)
  { 
    lastWiFiTry = timestamp;
    WiFi.disconnect() ;
    WiFi.begin ( ssid, password );  
  }

  //////////////////////////////////////////////////////////////
  // outside temperature, received by RF433?
  //////////////////////////////////////////////////////////////
  int protocol;
  unsigned long code;
  if ( rcv.receive(protocol, code) )
  {
    // Serial.print("Protocol: ");
    // Serial.print(protocol);
    // Serial.print(", code:");
    // Serial.print(code, BIN);
    // Serial.print(" / ");
    // Serial.print(code, HEX);
    // Serial.println();
    if ( protocol == WEATHERSTATION)
    {
      ctrl.outsideTimestamp = timestamp;
      int temp = receiver::convertCodeToTemp(code);
      if ( ctrl.outsideTemperature != temp )
      {
        ctrl.outsideTemperature = temp;
        screen.TriggerUpdate();
      }
    }
  }
  if ( ctrl.outsideTemperature != INVALID_TEMP && timestamp - ctrl.outsideTimestamp > TEMP_VALIDITY)
  {
    ctrl.outsideTemperature = INVALID_TEMP;
  }

  //////////////////////////////////////////////////////////////
  // Communication from pump controller
  //////////////////////////////////////////////////////////////
  if ( communicator.Read() )
  {
    ctrl.LastValidPumpTimestamp = timestamp;
    ctrl.pumpCommunicationOK = true;
    if ( ctrl.waterTemperature != communicator.m_temperature )
    {
      controlValuesChanged = true;
      if ( communicator.m_temperature > 0 ) ctrl.waterTemperature = communicator.m_temperature;
      else                                  ctrl.waterTemperature = INVALID_TEMP;
    }
    if ( ctrl.isPumpForced != communicator.m_pumpForcedOn )
    {
      ctrl.isPumpForced = communicator.m_pumpForcedOn;
      if ( ctrl.isPumpForced )
      {
        ctrl.lastforcedon = timestamp;
      }
    }
    if ( ctrl.isPumpOn != communicator.m_pumpOn )
    {
      ctrl.isPumpOn = communicator.m_pumpOn;
      sendToPump = true;  // It's not what we expected, so we might have to update the pump controller.
    }
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.println("Received update from pump controller"));
    screen.TriggerUpdate();
  }
  
  if ( ctrl.pumpCommunicationOK && timestamp - ctrl.LastValidPumpTimestamp > PUMP_VALIDITY )
  {
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.print("Invalidated pump communication "));
    DEBUGONLY(Serial.print(timestamp));
    DEBUGONLY(Serial.print(" "));
    DEBUGONLY(Serial.println(ctrl.LastValidPumpTimestamp));
    ctrl.pumpCommunicationOK = false;
    ctrl.waterTemperature = INVALID_TEMP;
    screen.TriggerUpdate();
  }

  timestamp = millis(); // Freeze the time again, communication might have been lengthy

  //////////////////////////////////////////////////////////////
  // Communication with the OpenTherm gateway
  //////////////////////////////////////////////////////////////
  if ( false ) //timestamp - lastOTOutsideTemp > OTOUTSIDE_INTERVAL )
  {
    WiFiClient client;
    HTTPClient http;
    http.setTimeout(500);
    http.begin(client, "http://192.168.178.6/read?id=27");
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK ) {
      lastOTOutsideTemp = timestamp;
      String payload = http.getString();
      auto idx = payload.indexOf('=');
      if ( idx > 0 )
      {
        ctrl.outsideTemperatureOT = atof(payload.c_str() + idx + 1) * 10;
        lastOTOutsideTempOK = timestamp;
        screen.TriggerUpdate();
      }
    }
    else if ( httpCode == 202 )
    {
      lastOTOutsideTemp = timestamp - OTOUTSIDE_INTERVAL + 2000; // Requested. Get the value after 2 second.
    }
    else
    {
      lastOTOutsideTemp = timestamp - OTOUTSIDE_INTERVAL + OTOUTSIDE_RETRY;
    }
    http.end();
  }
  if ( timestamp - lastOTOutsideTempOK > OTOUTSIDE_INVALID_TIMEOUT )
  {
    ctrl.outsideTemperatureOT = INVALID_TEMP;
  }

  timestamp = millis(); // Freeze the time again, communication might have been lengthy

  //////////////////////////////////////////////////////////////
  // Inside temperature, measured locally
  //////////////////////////////////////////////////////////////
  if ( ctrl.insideRequested )
  {
    if ( timestamp - ctrl.insideTimestamp > DS18B20_DELAY )
    {
      ctrl.insideRequested = false;
      ctrl.insideTimestamp = timestamp;
      int temp = insidetemp.getTempCByIndex(0) * 10;
      if ( temp < -100 )
      {
        temp = INVALID_TEMP;
        DEBUGONLY(LogTime());
        DEBUGONLY(Serial.println("Invalid inside temperature"));
      }
      if ( temp != ctrl.insideTemperature )
      {
        ctrl.insideTemperature = temp;
        screen.TriggerUpdate();
        controlValuesChanged = true;
        DEBUGONLY(LogTime());
        DEBUGONLY(Serial.print("Inside temp changed to "));
        DEBUGONLY(Serial.println(ctrl.insideTemperature));
      }
    }
  }
  else
  {
    if ( timestamp - ctrl.insideTimestamp > MEASURE_INTERVAL )
    {
      insidetemp.requestTemperatures();
      ctrl.insideRequested = true;
      ctrl.insideTimestamp = timestamp;
      DEBUGONLY(LogTime());
      DEBUGONLY(Serial.println("Requested inside temperature"));
    }
  }

  //////////////////////////////////////////////////////////////
  // Update the display for the user.
  //////////////////////////////////////////////////////////////
  auto oldsetpoint = ctrl.insideTemperatureSetpoint;
  screen.Proces();

  //////////////////////////////////////////////////////////////
  // The actual controlling actions.
  //////////////////////////////////////////////////////////////

  if ( ctrl.insideSetpointDuration > 0 )
  {
    ctrl.insideSetpointDuration -= min(ctrl.insideSetpointDuration, timestamp - lastloopmillis);
    if ( ctrl.insideSetpointDuration == 0 )
    {
      ctrl.insideTemperatureSetpoint = DEFAULT_INSIDE_TEMP_SETPOINT;
    }
  }

  // Either by UI actions or the time-out above.
  controlValuesChanged |= oldsetpoint != ctrl.insideTemperatureSetpoint;

  if ( controlValuesChanged && ControlValuesAreValid() )
  {
    if ( ctrl.pumpNeedsOn )
    {
      if ( ctrl.insideTemperature > ctrl.insideTemperatureSetpoint + 1 || ctrl.waterTemperature <= ctrl.waterTemperatureSetpoint )
      {
        ctrl.pumpNeedsOn = false;
        screen.TriggerUpdate();
        sendToPump = true;
      }
    }
    else
    {
      if ( ctrl.insideTemperature <= ctrl.insideTemperatureSetpoint - 1 && ctrl.waterTemperature >= ctrl.waterTemperatureSetpoint + 10 )
      {
        ctrl.pumpNeedsOn = true;
        screen.TriggerUpdate();
        sendToPump = true;
      }
    }      
  }
  
  //////////////////////////////////////////////////////////////
  // Communication to the pump controller
  //////////////////////////////////////////////////////////////
  if ( !sendToPump && timestamp - ctrl.pumpSendTimestamp > MINIMUM_COMMUNICATION_INTERVAL )
  {
    sendToPump = true;
  }

  // If needed, communicate our status to the pump. But only if we have something valid to tell.
  if ( sendToPump && ControlValuesAreValid() )
  {
    communicator.Send(ctrl.waterTemperatureSetpoint, ctrl.pumpNeedsOn, ctrl.isPumpForced /* ignored by pump */ );
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.println(F("Send update to pump.")));
    ctrl.pumpSendTimestamp = timestamp;
  }

  //////////////////////////////////////////////////////////////
  // Prevent a power-sucking 100% CPU loop.
  //////////////////////////////////////////////////////////////
  delay(20);
  lastloopmillis = timestamp;
}
