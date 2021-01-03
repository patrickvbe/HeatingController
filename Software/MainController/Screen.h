#include <OLEDDisplay.h>
#include "ControlValues.h"

class Screen;

class DisplayMode
{
  public:
    DisplayMode(Screen&);
    virtual void Enter() {}
    virtual void B1Down() {}
    virtual void B1LongDown() {}
    virtual void B1Up(bool longdown) {}
    virtual void B2Down() {}
    virtual void B2LongDown() {}
    virtual void B2Up(bool longdown) {}
    virtual void Display() {}
    virtual void TickSecond() {}

  protected:
    Screen& MScreen;
};

class DMMain: public DisplayMode
{
  public:
    DMMain(Screen& screen) : DisplayMode(screen) {}
    void B1Down() override;
    void B2Down() override;
    void Display() override;
};

class DMStatus1: public DisplayMode
{
  public:
    DMStatus1(Screen& screen) : DisplayMode(screen) {}
    void B2Down() override;
    void Display() override;
    void TickSecond() override;
};

class DMStatus2: public DisplayMode
{
  public:
    DMStatus2(Screen& screen) : DisplayMode(screen) {}
    void B2Down() override;
    void Display() override;
};

class DMChangeTD: public DisplayMode
{
  public:
    DMChangeTD(Screen& screen) : DisplayMode(screen) {}
    void Enter() { MJustEntered = true; }
    void Display() override;
    void TickSecond() override;
  protected:
    bool MJustEntered = true;
};

class DMChangeTemp: public DMChangeTD
{
  public:
    DMChangeTemp(Screen& screen) : DMChangeTD(screen) {}
    void B1LongDown() override;
    void B1Up(bool longpress) override;
    void B2Down() override;
    void Display() override;
  protected:
    void ResetTime();
};

class DMChangeDuration: public DMChangeTD
{
  public:
    DMChangeDuration(Screen& screen) : DMChangeTD(screen) {}
    void B1LongDown() override;
    void B1Up(bool longpress) override;
    void B2LongDown() override;
    void B2Up(bool longpress) override;
    void Display() override;
};

class Screen
{
  public:
    Screen(OLEDDisplay& display, ControlValues& ctrl, byte b1pin, byte b2pin);
    void Proces();
    void Enter(DisplayMode& mode);
    void TriggerUpdate() { MDoUpdate = true; }

    OLEDDisplay&    MDisplay;
    ControlValues&  MCtrl;

    DMMain            dmmain;
    DMStatus1         dmstatus1;
    DMStatus2         dmstatus2;
    DMChangeTemp      dmchangetemp;
    DMChangeDuration  dmchangeduration;

  private:
    void Display();

    DisplayMode*    MPMode;
    unsigned long   MDownstart=0;
    unsigned long   MLastTick=0;
    byte            B1pin;
    byte            B2pin;
    bool            MButton1Down = false;
    bool            MButton2Down = false;
    bool            MLongDownCalled = false;
    bool            MDoUpdate = true;
};
