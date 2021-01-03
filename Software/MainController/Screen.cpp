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
      MLongDownCalled = false;
      MPMode->B1Down();
    }
    else
    {
      MPMode->B1Up(MLongDownCalled);
    }
  }
  if ( (digitalRead(B2pin) == LOW) != MButton2Down )
  {
    if ( (MButton2Down = !MButton2Down) )
    {
      MDownstart = timestamp;
      MLongDownCalled = false;
      MPMode->B2Down();
    }
    else
    {
      MPMode->B2Up(MLongDownCalled);
    }
  }
  if ( MButton1Down && !MLongDownCalled && timestamp - MDownstart > 1000 )
  {
    MPMode->B1LongDown();
    MLongDownCalled = true;
  }
  else if ( MButton2Down && !MLongDownCalled && timestamp - MDownstart > 1000 )
  {
    MPMode->B2LongDown();
    MLongDownCalled = true;
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
    Display();
  }
}

void Screen::Enter(DisplayMode& mode)
{
  MPMode = &mode;
  MPMode->Enter();
  Display();
}

void Screen::Display()
{
  MDisplay.clear();
  MPMode->Display();
  MDisplay.display();
  MDoUpdate = false;
}

/////////////////////////////////////////////////////////////////////

DisplayMode::DisplayMode(Screen& screen)
: MScreen(screen)
{
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
  PrintString str;

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Open_Sans_Condensed_Bold_30);
  str.ConcatTemp(ctrl.insideTemperature);
  display.drawString(0, 0, str);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  str = "";
  str.ConcatTemp(ctrl.outsideTemperature);
  display.drawString(128, 0, str);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  str = "CV: ";
  str.ConcatTemp(ctrl.waterTemperature, false);
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
  PrintString str;
  auto timestamp = millis();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  
  display.drawString(0, line, "buiten");
  str = SColon;
  str.ConcatTime(timestamp - ctrl.outsideTimestamp);
  display.drawString(colonposition, line, str);
  
  display.drawString(0, line += lineheight, "binnen");
  str = SColon;
  str.ConcatTime(timestamp - ctrl.insideTimestamp);
  display.drawString(colonposition, line, str);
  
  display.drawString(0, line += lineheight, "p-comm");
  str = SColon;
  str.ConcatTime(timestamp - ctrl.LastValidPumpTimestamp);
  display.drawString(colonposition, line, str);
  
  display.drawString(0, line += lineheight, "periodiek");
  str = SColon;
  if ( (timestamp - ctrl.lastforcedon) > (24 * 3600 * 1000) ) str.concat("> dag");
  else str.ConcatTime(timestamp - ctrl.lastforcedon);
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
  PrintString str;

  display.drawString(0, line, "water");
  str = SColon;
  str.ConcatTemp(ctrl.waterTemperature);
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

void DMChangeTD::Display()
{
  auto& display = MScreen.MDisplay;
  auto& ctrl    = MScreen.MCtrl;
  PrintString str;

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Open_Sans_Condensed_Bold_30);
  str.ConcatTemp(ctrl.insideTemperatureSetpoint);
  display.drawString(0, 0, str);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  str = "";
  str.ConcatTime( (ctrl.insideSetpointDuration + 59999) / 60);
  display.drawString(128, 0, str);
}

void DMChangeTD::TickSecond()
{
  MScreen.TriggerUpdate();
}

/////////////////////////////////////////////////////////////////////

void DMChangeTemp::B1LongDown()
{
  MScreen.Enter(MScreen.dmchangeduration);
}

void DMChangeTemp::B1Up(bool longpress)
{
  if ( MJustEntered )
  {
    MJustEntered = false;
  }
  else
  {
    MScreen.MCtrl.insideTemperatureSetpoint += 5;
    ResetTime();
  }  
}

void DMChangeTemp::B2Down()
{
    MScreen.MCtrl.insideTemperatureSetpoint -= 5;
    ResetTime();
}

void DMChangeTemp::ResetTime()
{
  if ( MScreen.MCtrl.insideSetpointDuration == 0 )
  {
    MScreen.MCtrl.insideSetpointDuration = 10 * 3600 * 1000;
  }
  MScreen.TriggerUpdate();
}

void DMChangeTemp::Display()
{
  DMChangeTD::Display();
  MScreen.MDisplay.drawLine(0,0,64,0);
}

/////////////////////////////////////////////////////////////////////

void DMChangeDuration::B1LongDown()
{
  MScreen.Enter(MScreen.dmmain);
}

void DMChangeDuration::B1Up(bool longpress)
{
  if ( MJustEntered )
  {
    MJustEntered = false;
  }
  else
  {
    MScreen.MCtrl.insideSetpointDuration += 1800000;
    MScreen.TriggerUpdate();
  }  
}

void DMChangeDuration::B2LongDown()
{
  MScreen.MCtrl.insideSetpointDuration = 0; // = never change back.
}

void DMChangeDuration::B2Up(bool longpress)
{
  MScreen.MCtrl.insideSetpointDuration -= min(MScreen.MCtrl.insideSetpointDuration, 1800000UL);
  MScreen.TriggerUpdate();
}

void DMChangeDuration::Display()
{
  DMChangeTD::Display();
  MScreen.MDisplay.drawLine(64,0,128,0);
}
