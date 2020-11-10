#include "Screen.h"

Screen::Screen(OLEDDisplay& display, ControlValues& ctrl, byte b1pin, byte b2pin)
: MDisplay(display), MCtrl(ctrl), B1pin(b1pin), B2pin(b2pin),
  mode00(*this), mode01(*this), mode02(*this),
  mode11(*this), mode12(*this)
{
  MPMode = &mode00;
  pinMode(b1pin, INPUT);
  pinMode(b2pin, INPUT);
}

void Screen::Proces()
{
  unsigned long timestamp = millis();
  if ( (digitalRead(B1pin) == LOW) != MButton1Down )
  {
    TriggerUpdate();
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
    TriggerUpdate();
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
  if ( MPMode != &mode00 && timestamp - MDownstart > 10000 )
  {
    MPMode = &mode00;
    TriggerUpdate();
  }
  // Update the screen, if needed.
  if ( MDoUpdate )
  {
    MPMode->Display();
    MDoUpdate = false;
  }
}

void Screen::Enter(DisplayMode& mode)
{
  MPMode = &mode;
  MPMode->Enter();
  TriggerUpdate();
}

/////////////////////////////////////////////////////////////////////

DisplayMode::DisplayMode(Screen& screen)
: MScreen(screen)
{
}

/////////////////////////////////////////////////////////////////////
void DisplayMode00::B1Down()
{
  MScreen.Enter(MScreen.mode10);
};

void DisplayMode00::B2Down()
{
  MScreen.Enter(MScreen.mode01);
};

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
