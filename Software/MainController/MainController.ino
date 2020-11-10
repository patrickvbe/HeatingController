// Going OTA...
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
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

#define DEBUG;
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
#define BUTTON1_PIN 2
#define BUTTON2_PIN 0
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

// WiFi
WiFiEventHandler mConnectHandler, mDisConnectHandler, mGotIpHandler;
const unsigned long WIFI_TRY_INTERVAL = 60000;
unsigned long lastWiFiTry = -WIFI_TRY_INTERVAL;

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

// Fonts
#include "Open_Sans_Condensed_Bold_30.h"

#include "Screen.h"
Screen screen(display, ctrl, BUTTON1_PIN, BUTTON2_PIN);

static char* SColon = ": ";

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
void ShowFullScreenStatus(char* status)
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Open_Sans_Condensed_Bold_30);
  display.drawString(0, 0, status);
  display.display();
}

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
    ShowFullScreenStatus("OTA");
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
    ShowFullScreenStatus("ota failed");
    delay(1000);
    rcv.start();
    doingota = false;
  });

  //////////////////////////////////////////////////////////////
  // 
  //////////////////////////////////////////////////////////////
  rcv.start();
  insidetemp.begin();
  insidetemp.setResolution(12);
  insidetemp.setWaitForConversion(false);
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
 * String formatting class, compatible with the Print interface
 ***************************************************************/
class PrintString : public Print, public String
{
public:
  PrintString() : Print(), String() {}
  PrintString(const char* s) : Print(), String(s) {}
  virtual size_t write(uint8_t c) { concat((char)c); }
  virtual size_t write(const uint8_t *buffer, size_t size)
  {
    reserve(length() + size + 1);
    while(size--) concat((char)*buffer++);
  }
  virtual int availableForWrite() { return 100; } // Whatever??
};

/***************************************************************
 * Format a temperature in 10ths of degrees to a nice string.
 ***************************************************************/
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
  bool controlValuesChanged = false;    // We only want to calculate the logic when input values changed (a bit over-the-top...)
  bool sendToPump = false;              // Do we need to update the pump controller?

  //////////////////////////////////////////////////////////////
  // OTA
  //////////////////////////////////////////////////////////////
  ArduinoOTA.handle();
  if ( doingota ) return;

  timestamp = millis(); // Freeze the time

  //////////////////////////////////////////////////////////////
  // Try to stay connected to the WiFi.
  //////////////////////////////////////////////////////////////
  if (WiFi.status() != WL_CONNECTED && wifiStatus != '#' && timestamp - lastWiFiTry > WIFI_TRY_INTERVAL)
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
      screen.TriggerUpdate();
      if ( communicator.m_temperature > 0 ) ctrl.waterTemperature = communicator.m_temperature;
      else                                  ctrl.waterTemperature = INVALID_TEMP;
    }
    if ( ctrl.isPumpForced != communicator.m_pumpForcedOn )
    {
      ctrl.isPumpForced = communicator.m_pumpForcedOn;
      screen.TriggerUpdate();
      if ( ctrl.isPumpForced )
      {
        ctrl.lastforcedon = timestamp;
      }
    }
    if ( ctrl.isPumpOn != communicator.m_pumpOn )
    {
      screen.TriggerUpdate();
      ctrl.isPumpOn = communicator.m_pumpOn;
      sendToPump = true;  // It's not what we expected, so we might have to update the pump controller.
    }
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.println("Received update from pump controller"));
  }
  
  if ( ctrl.pumpCommunicationOK && timestamp - ctrl.LastValidPumpTimestamp > PUMP_VALIDITY )
  {
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.print("Invalidated pump communication "));
    DEBUGONLY(Serial.print(timestamp));
    DEBUGONLY(Serial.print(" "));
    DEBUGONLY(Serial.println(LastValidPumpTimestamp));
    ctrl.pumpCommunicationOK = false;
    ctrl.waterTemperature = INVALID_TEMP;
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
        DEBUGONLY(Serial.println(insideTemperature));
      }
    }
  }
  else
  {
    if ( timestamp - ctrl.insideTimestamp > MEASURE_INTERVAL )
    {
      insidetemp.requestTemperatures(); // takes about 3/4s
      ctrl.insideRequested = true;
      ctrl.insideTimestamp = timestamp;
      DEBUGONLY(LogTime());
      DEBUGONLY(Serial.println("Requested inside temperature"));
    }
  }

  //////////////////////////////////////////////////////////////
  // The actual controlling actions.
  //////////////////////////////////////////////////////////////
  if ( ctrl.insideSetpointDuration != 0 && timestamp - ctrl.insideSetpointStart > ctrl.insideSetpointDuration )
  {
    ctrl.insideSetpointDuration = 0;
    ctrl.insideTemperatureSetpoint = DEFAULT_INSIDE_TEMP_SETPOINT;
  }

  if ( controlValuesChanged && ControlValuesAreValid() )
  {
    if ( ctrl.pumpNeedsOn )
    {
      if ( ctrl.insideTemperature >= ctrl.insideTemperatureSetpoint + 5 || ctrl.waterTemperature <= ctrl.waterTemperatureSetpoint )
      {
        ctrl.pumpNeedsOn = false;
        screen.TriggerUpdate();
        ctrl.sendToPump = true;
      }
    }
    else
    {
      if ( ctrl.insideTemperature <= ctrl.insideTemperatureSetpoint && ctrl.waterTemperature >= ctrl.waterTemperatureSetpoint + 10 )
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

/***************************************************************
 * Display modes:
 * 00: Show temperatures
 * 01: Show info screen 1
 * 02: Show info screen 2
 * 10: Modify inside temperature setpoint (in 10th C)
 * 11: Modify inside temperature duration (in hours)
 ***************************************************************/

  //////////////////////////////////////////////////////////////
  // Button control, still needs some de-bouncing
  //////////////////////////////////////////////////////////////
  // Bottom button (white) = display mode
  if ( (digitalRead(BUTTON1_PIN) == LOW) != Button1Down )
  {
    if ( (Button1Down = !Button1Down) )
    {
      if      ( displaymode < 10 && ++displaymode > 2 ) displaymode = 0;
      if      ( displaymode == 10 ) 
      { 
        if ( insideSetpointDuration == 0 )
        insideSetpointStart = timestamp; insideTemperatureSetpoint--; }
      else if ( displaymode == 11 ) { insideSetpointStart = timestamp; insideSetpointDuration -= min(insideSetpointDuration, 3600000); }
    }
    displayChanged = true;
  }
  // Top button (red) = settings mode
  if ( (digitalRead(BUTTON2_PIN) == LOW) != Button2Down )
  {
    if ( (Button1Down = !Button1Down) )
    {
      buttonStartTime = timestamp;
      if ( displaymode < 10 ) displaymode = 10;
    }
    else
    {
      bool longpress = timestamp - buttonStartTime > 1000;
      if      ( displaymode == 10 && longpress ) displaymode = 11;
      else if ( displaymode == 11 && longpress ) displaymode = 0;
      else if ( displaymode == 10 ) { insideSetpointStart = timestamp; insideTemperatureSetpoint++; }
      else if ( displaymode == 11 ) { insideSetpointStart = timestamp; insideSetpointDuration += 3600000; }
    }
    displayChanged = true;
  }
  // Reset display after 10 seconds of no button changes
  if ( displaymode != 0 && timestamp - buttonStartTime > 10000 )
  {
    displaymode = 0;
    displayChanged = true;
  }

  if ( displaymode == 1 && (timestamp - displayupdated) > 1000 ) displayChanged = true;

  //////////////////////////////////////////////////////////////
  // Update the display for the user.
  //////////////////////////////////////////////////////////////
  if ( displayChanged )
  {
    const word colonposition = 65;
    const word lineheight = 15;
    word line = 0;
    String str;
    displayupdated = timestamp;
    display.clear();
    if ( displaymode == 0 )
    {
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      //display.setFont(Open_Sans_Condensed_Bold_40);
      display.setFont(Open_Sans_Condensed_Bold_30);
      //display.setFont(ArialMT_Plain_24);
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
      str = wifiStatus;
      display.drawString(60, 46, str);
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
    }
    else if ( displaymode == 1 )
    {
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_16);
      
      display.drawString(0, line, "buiten");
      str = SColon;
      str.concat( (timestamp - outsideTimestamp) / 1000 );
      display.drawString(colonposition, line, str);
      
      display.drawString(0, line += lineheight, "binnen");
      str = SColon;
      str.concat( (timestamp - insideTimestamp) / 1000 );
      display.drawString(colonposition, line, str);
      
      display.drawString(0, line += lineheight, "p-comm");
      str = SColon;
      str.concat( (timestamp - LastValidPumpTimestamp) / 1000 );
      display.drawString(colonposition, line, str);
      
      display.drawString(0, line += lineheight, "periodiek");
      str = SColon;
      if ( (timestamp - lastforcedon) > (24 * 3600 * 1000) ) str.concat("> dag");
      else str.concat( (timestamp - lastforcedon) / 1000 );
      display.drawString(colonposition, line, str);
    }
    else if ( displaymode == 2 )
    {
      display.drawString(0, line, "water");
      str = SColon;
      ConcatTemp(waterTemperature, str);
      display.drawString(colonposition, line, str);

      display.drawString(0, line += lineheight, "pomp");
      str = SColon;
      str.concat(!isPumpOn ? "uit" : isPumpForced ? "(p)" : "aan");
      display.drawString(colonposition, line, str);

      display.drawString(0, line += lineheight, "comm.");
      str = SColon;
      str.concat(pumpCommunicationOK ? "OK" : "Fout");
      display.drawString(colonposition, line, str);

      PrintString pstr;
      pstr.print(WiFi.localIP());
      display.drawString(0, line += lineheight, pstr);
    }
    display.display();
    displayChanged = false;
  }

  //////////////////////////////////////////////////////////////
  // Prevent a power-sucking 100% CPU loop.
  //////////////////////////////////////////////////////////////
  delay(20);
}
