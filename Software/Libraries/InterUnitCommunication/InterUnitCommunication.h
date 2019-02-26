
// Own protocol for controlling the remote pump and getting response back.
class InterUnitCommunication
{
  public:
    InterUnitCommunication(unsigned long received_code)
    {
      Code(received_code);
    }

    void Code(unsigned long received_code)
    {
      code         = received_code;
      unitCode     = (code >> 12) & 0xFFF;      // 12 bit unit code
      temperature  = ((code >>  4) & 0xFF) * 5; //  8 bit temperature in half degrees
      pumpOn       = (code & 8) != 0;           //  1 bit pump status
      pumpForcedOn = (code & 4) != 0;           //  1 bit pump-is-forced
      isValid      = (code & 3) == CalcChecksum(code); // 2-bit 'checksum'
    }

    static unsigned long CalculateCode(const int unitCode,
                                      const int temperature /* in tenths of C */,
                                      bool pumpOn, bool pumpForced)
    {
      unsigned long code = (unitCode << 12) | (temperature / 5) << 4 | pumpOn ? 8 : 0 | pumpForced ? 4 : 0;
      return code | CalcChecksum(code);
    }

    static unsigned long CalcChecksum(unsigned long code)
    {
      unsigned long value = code;
      unsigned long poorcrc = 0;
      for ( int i = 11; i > 0; i--)
      {
        value >>= 2;
        poorcrc ^= value;
      }
      return poorcrc | 3;
    }

    unsigned long code;   // The code received
    int   unitCode;       // Unique code to identify the unit
    int   temperature;    // The temperature in tenths of Celcius (transmitted as half degrees possitive only)
    bool  pumpOn;         // Is / should the pump (be) on.
    bool  pumpForcedOn;   // Is the pump forcibly on (because it hasn't run for x hours). Only from pump to controller.
    bool  isValid;        // Was the received signal valid?
};
