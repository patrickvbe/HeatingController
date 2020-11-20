#include "Screen.h"

// Fonts
#include "Open_Sans_Condensed_Bold_30.h"

// Various
#include <ESP8266WiFi.h>
#include "PrintString.h"

static char* SColon = ": ";

Screen::Screen(OLEDDisplay& display, ControlValues& ctrl, byte b1pin, byte b2pin)
: MDisplay(display), MCtrl(ctrl), B1pin(b1pin), B2pin(b2pin),
  dmmain(*this), dmstatus1(*this), dmstatus2(*this),
  dmchangetemp(*this), dmchangeduration(*this)
{
  MPMode = &dmmain;
  pinMode(b1pin, INPUT);
  pinMode(b2pin, INPUT);
}

void Screen::Proces()
{
  unsigned long timestamp = millis();

  if ( (digitalRead(B1pin) == LOW) != MButton1Down )
  {
    if ( (MButton1Down = !MButton1Down) )
    {
      MDownstart = timestamp;
      MPMode->B1Down();
    }
    else
    {
      MPMode->B1Up(timestamp - MDownstart > 1000);
    }
  }
  if ( (digitalRead(B2pin) == LOW) != MButton2Down )
  {
    if ( (MButton2Down = !MButton2Down) )
    {
      MDownstart = timestamp;
      MPMode->B2Down();
    }
    else
    {
      MPMode->B2Up(timestamp - MDownstart > 1000);
    }
  }
  // Reset display after 10 seconds of no button changes
  if ( MPMode != &dmmain && timestamp - MDownstart > 10000 )
  {
    Enter(dmmain);
  }

  // Give every second a pulse to allow times updates.
  if ( timestamp - MLastTick > 1000 )
  {
    MPMode->TickSecond();
    MLastTick = timestamp;
  }

  // Update the screen, if needed.
  if ( MDoUpdate )
  {
    MDisplay.clear();
    MPMode->Display();
    MDisplay.display();
    MDoUpdate = false;
  }
}

void Screen::Enter(DisplayMode& mode)
{
  MPMode = &mode;
  MPMode->Enter();
  MPMode->Display();
}

/////////////////////////////////////////////////////////////////////

DisplayMode::DisplayMode(Screen& screen)
: MScreen(screen)
{
}

/***************************************************************
 * Format a temperature in 10ths of degrees to a nice string.
 ***************************************************************/
void DisplayMode::ConcatTemp(int temp, String& str)
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

/////////////////////////////////////////////////////////////////////
void DMMain::B1Down()
{
  MScreen.Enter(MScreen.dmchangetemp);
};

void DMMain::B2Down()
{
  MScreen.Enter(MScreen.dmstatus1);
};

void DMMain::Display()
{
  auto& display = MScreen.MDisplay;
  auto& ctrl    = MScreen.MCtrl;
  String str;

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Open_Sans_Condensed_Bold_30);
  ConcatTemp(ctrl.insideTemperature, str);
  display.drawString(0, 0, str);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  str = "";
  ConcatTemp(ctrl.outsideTemperature, str);
  display.drawString(128, 0, str);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  str = "CV: ";
  ConcatTemp(ctrl.waterTemperature, str);
  display.drawString(0, 46, str);
  str = ctrl.wifiStatus;
  display.drawString(60, 46, str);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  if ( ctrl.pumpCommunicationOK )
  {
    str = "P ";
    if ( ctrl.isPumpForced )
    {
      str.concat("AAN");
    }
    else
    {
      str.concat(ctrl.isPumpOn ? "aan" : "uit");
      if ( ctrl.pumpNeedsOn != ctrl.isPumpOn ) str.concat("!");
    }
  }
  else
  {
    str = "<fout>";
  }
  display.drawString(128, 46, str);
}

/////////////////////////////////////////////////////////////////////

void DMStatus1::B2Down()
{
  MScreen.Enter(MScreen.dmstatus2);
};

void DMStatus1::Display()
{
  auto& display = MScreen.MDisplay;
  auto& ctrl    = MScreen.MCtrl;
  const word colonposition = 65;
  const word lineheight = 15;
  word line = 0;
  String str;
  auto timestamp = millis();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  
  display.drawString(0, line, "buiten");
  str = SColon;
  str.concat( (timestamp - ctrl.outsideTimestamp) / 1000 );
  display.drawString(colonposition, line, str);
  
  display.drawString(0, line += lineheight, "binnen");
  str = SColon;
  str.concat( (timestamp - ctrl.insideTimestamp) / 1000 );
  display.drawString(colonposition, line, str);
  
  display.drawString(0, line += lineheight, "p-comm");
  str = SColon;
  str.concat( (timestamp - ctrl.LastValidPumpTimestamp) / 1000 );
  display.drawString(colonposition, line, str);
  
  display.drawString(0, line += lineheight, "periodiek");
  str = SColon;
  if ( (timestamp - ctrl.lastforcedon) > (24 * 3600 * 1000) ) str.concat("> dag");
  else str.concat( (timestamp - ctrl.lastforcedon) / 1000 );
  display.drawString(colonposition, line, str);
}

void DMStatus1::TickSecond()
{
  MScreen.TriggerUpdate();
}

/////////////////////////////////////////////////////////////////////

void DMStatus2::B2Down()
{
  MScreen.Enter(MScreen.dmmain);
};

void DMStatus2::Display()
{
  auto& display = MScreen.MDisplay;
  auto& ctrl    = MScreen.MCtrl;
  const word colonposition = 65;
  const word lineheight = 15;
  word line = 0;
  String str;

  display.drawString(0, line, "water");
  str = SColon;
  ConcatTemp(ctrl.waterTemperature, str);
  display.drawString(colonposition, line, str);

  display.drawString(0, line += lineheight, "pomp");
  str = SColon;
  str.concat(!ctrl.isPumpOn ? "uit" : ctrl.isPumpForced ? "AAN" : "aan");
  display.drawString(colonposition, line, str);

  display.drawString(0, line += lineheight, "comm.");
  str = SColon;
  str.concat(ctrl.pumpCommunicationOK ? "OK" : "Fout");
  display.drawString(colonposition, line, str);

  PrintString pstr;
  pstr.print(WiFi.localIP());
  display.drawString(0, line += lineheight, pstr);
}

/////////////////////////////////////////////////////////////////////
