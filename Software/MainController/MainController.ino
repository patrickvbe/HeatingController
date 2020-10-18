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
  #define DEBUGONLY()
#endif

// IO5 = D1
// IO4 = D2
// IO13 = D7
// IO0 = D3, has pull-up resistor
// IO2 = D4, has pull-up resistor
#define SENDER_PIN    5
#define RECEIVER_PIN  4
#define DS18B20_PIN  13
#define BUTTON1_PIN 0
#define BUTTON2_PIN 2

const int INVALID_TEMP =       -1000;

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
#define MEASURE_INTERVAL                10000
#define FORCE_TIME_DURATION            300000
#endif

// Display
// IO12 = D6
// IO14 = D5
#define DISPLAY_ADDRESS 0x3C
#define SDA_PIN 12
#define SCL_PIN 14
#include <SH1106Wire.h>
//#include <SSD1306Wire.h>
#include "Open_Sans_Condensed_Bold_30.h"
//SSD1306Wire  display(DISPLAY_ADDRESS, SDA_PIN, SCL_PIN);
SH1106Wire  display(DISPLAY_ADDRESS, SDA_PIN, SCL_PIN);

// The values we preserve
unsigned long LastValidPumpTimestamp = 0;                   // The last time we received valid information from the pump unit.
unsigned long pumpSendTimestamp = 0;                        // Timestamp of last information send to the pump unit.
int           waterTemperature  = INVALID_TEMP;             // CV water temperature received from the pump unit.
bool          isPumpOn = false;                             // Pump status received from the pump unit.
bool          isPumpForced = false;                         // Forced status received from the pump unit.
bool          pumpNeedsOn = false;                          // Our computed / wanted pump status.
bool          pumpCommunicationOK = false;                  // Did we receive valid and on-time communication from the pump unit?
bool          sendToPump = false;                           // Do we need to update the pump controller?
int           outsideTemperature = INVALID_TEMP;            // New temperature received from the weather station.
int           outsidePreviousTemperature = INVALID_TEMP;    // Previous outside temperature (now displayed)
int           insideTemperature  = INVALID_TEMP;            // Last measured inside temperature.
int           waterTemperatureSetpoint = 220;               // CV water temperature setpoint to turn the pump on / off. Might be configurable in the future.
int           insideTemperatureSetpoint = 215;              // Room temperature setpoint to turn the pump on / off. Might be configurable in the future.
int           displaymode = 0;                              // What to show on the screen
unsigned long outsideTimestamp  = 0;                        // Timestamp of the last valid outside temperature received.
unsigned long insideTimestamp = 0;                          // Timestamp the inside temperature was last measured.
unsigned long timestamp;                                    // The globall frozen time.
unsigned long lastforcedon=0;
unsigned long displayupdated=0;
bool          Button1Down = false;
bool          Button2Down = false;

static char* SColon = ": ";

// Interrupt routine for RF433 communication.
void code_received(int protocol, unsigned long code, unsigned long timestamp)
{
  if ( protocol == WEATHERSTATION)
  {
    outsideTemperature = receiver::convertCodeToTemp(code);
    outsideTimestamp = timestamp;
    DEBUGONLY(Serial.print("Outside: "));
    DEBUGONLY(Serial.println(outsideTemperature));
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
receiver                rcv(RECEIVER_PIN, code_received);
sender                  snd(SENDER_PIN);
// RXD0 = IO3 = TX
// TXD0 = IO1 = RX
InterUnitCommunication  communicator;  // For now, uses the default serial port.
OneWire                 onewire(DS18B20_PIN);
DallasTemperature       insidetemp(&onewire);

void setup()
{
  Serial.begin(115200);

  // Display
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Open_Sans_Condensed_Bold_30);
  display.drawString(0, 0, "Booting");
  display.display();

  // OTA, bit rude currently, needs to boot without WiFi.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setHostname("HeatController");
  ArduinoOTA.onStart([]() {
    doingota = true;
    rcv.stop();
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(Open_Sans_Condensed_Bold_30);
    display.drawString(0, 0, "OTA");
    display.display();
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    rcv.start();
    doingota = false;
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
    rcv.start();
    doingota = false;
  });
  ArduinoOTA.begin();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Buttons
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);

  rcv.start();
  insidetemp.begin();
  insidetemp.setResolution(12);
}

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
  // OTA
  ArduinoOTA.handle();
  if ( doingota ) return;

  timestamp = millis();                 // Freeze the time
  bool displayChanged = false;          // We only want to update the display if displayed values changed.
  bool controlValuesChanged = false;    // We only want to calculate the logic when input values changed (a bit over-the-top...)

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
  if ( communicator.Read() )
  {
    LastValidPumpTimestamp = timestamp;
    pumpCommunicationOK = true;
    if ( waterTemperature != communicator.m_temperature )
    {
      controlValuesChanged = true;
      displayChanged = true;
      waterTemperature = communicator.m_temperature;
    }
    if ( isPumpForced != communicator.m_pumpForcedOn )
    {
      isPumpForced = communicator.m_pumpForcedOn;
      displayChanged = true;
      if ( isPumpForced )
      {
        lastforcedon = timestamp;
      }
    }
    if ( isPumpOn != communicator.m_pumpOn )
    {
      displayChanged = true;
      isPumpOn = communicator.m_pumpOn;
      sendToPump = true;  // It's not what we expected, so we might have to update the pump controller.
    }
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.println("Received update from pump controller"));
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
    if ( temp < -100 ) temp = INVALID_TEMP;
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
    communicator.Send(waterTemperatureSetpoint, pumpNeedsOn, isPumpForced /* ignored by pump */ );
    DEBUGONLY(LogTime());
    DEBUGONLY(Serial.println(F("Send update to pump.")));
    pumpSendTimestamp = timestamp;
    sendToPump = false;
  }

  // Button control
  if ( (digitalRead(BUTTON1_PIN) == LOW) != Button1Down )
  {
    if ( (Button1Down = !Button1Down) )
    {
      if ( ++displaymode > 2 ) displaymode = 0;
      displayChanged = true;
    }
  }
  if ( displaymode == 1 && (timestamp - displayupdated) > 1000 ) displayChanged = true;

  // Update the display for the user.
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
} // loop()
