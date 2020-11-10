#include <OLEDDisplay.h>
#include "ControlValues.h"

class DisplayMode;

class Screen
{
  public:
    Screen(OLEDDisplay& display, ControlValues& ctrl, byte b1pin, byte b2pin);
    void Proces();
    void Enter(DisplayMode& mode);
    void TriggerUpdate() { MDoUPdate = true; }

    OLEDDisplay&    MDisplay
    ControlValues&  MCtrl;

    DisplayMode00   mode00;
    DisplayMode01   mode01;
    DisplayMode02   mode02;
    DisplayMode10   mode10;
    DisplayMode11   mode11;

  private:
    DisplayMode*    MPMode;
    unsigned long   MDownstart=0;
    byte            B1pin;
    byte            B2pin;
    bool            MButton1Down = false;
    bool            MButton2Down = false;
    bool            MDoUpdate = true;
};

class DisplayMode
{
  public:
    DisplayMode(Screen&);
    virtual void Enter() {};
    virtual void B1Down() {};
    virtual void B1Up(bool longpress) {};
    virtual void B2Down() {};
    virtual void B2Up(bool longpress) {};
    virtual void Display() {};

  protected:
    Screen& MScreen;
};

class DisplayMode00: public DisplayMode
{
  public:
    DisplayMode00(Screen& screen) : DisplayMode(screen) {};
    virtual void B1Down() {};
    virtual void B2Down() {};
    virtual void Display() {};
};

class DisplayMode01: public DisplayMode
{
  public:
    DisplayMode01(Screen& screen) : DisplayMode(screen) {};
    virtual void B1Down() {};
    virtual void B1Up(bool longpress) {};
    virtual void B2Down() {};
    virtual void B2Up(bool longpress) {};
    virtual void Display() {};
};

class DisplayMode02: public DisplayMode
{
  public:
    DisplayMode02(Screen& screen) : DisplayMode(screen) {};
    virtual void B1Down() {};
    virtual void B1Up(bool longpress) {};
    virtual void B2Down() {};
    virtual void B2Up(bool longpress) {};
    virtual void Display() {};
};

class DisplayMode10: public DisplayMode
{
  public:
    DisplayMode10(Screen& screen) : DisplayMode(screen) {};
    virtual void B1Down() {};
    virtual void B1Up(bool longpress) {};
    virtual void B2Down() {};
    virtual void B2Up(bool longpress) {};
    virtual void Display() {};
};

class DisplayMode11: public DisplayMode
{
  public:
    DisplayMode11(Screen& screen) : DisplayMode(screen) {};
    virtual void B1Down() {};
    virtual void B1Up(bool longpress) {};
    virtual void B2Down() {};
    virtual void B2Up(bool longpress) {};
    virtual void Display() {};
};
