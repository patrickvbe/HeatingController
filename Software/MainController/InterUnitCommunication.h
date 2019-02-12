
// Own protocol for controlling the remote pump and getting response back.
class InterUnitCommunication
{
  InterUnitCommunication(unsigned long received_code)
  {
    code = (int)received_code;
    isValid = (code & 3) == CalcChecksum();
    temperature  = code >> 4;
    pumpOn       = (code & 8) != 0;
    pumpForcedOn = (code & 4) != 0;
  }

  InterUnitCommunication(int temperature /* in tenths of C */, bool pumpOn, bool pumpForced)
  {
    code = (temperature / 5) << 4 | pumpOn ? 8 : 0 | pumpForced ? 4 : 0;
    code |= CalcChecksum();
  }

  unsigned long CalcChecksum()
  {
    int value = code;
    int poorcrc = 0;
    for ( int i = 5; i > 0; i--)
    {
      value >>= 2;
      poorcrc ^= value;
    }
    return poorcrc | 3;
  }

  int   code;           // The code to send / received
  int   temperature;    // The temperature in tenths of Celcius (transmitted as half degrees possitive only)
  bool  pumpOn;         // Is / should the pump (be) on.
  bool  pumpForcedOn;   // Is the pump forcibly on (because it hasn't run for x hours). Only from pump to controller.
  bool  isValid;        // Was the received signal valid?
};
