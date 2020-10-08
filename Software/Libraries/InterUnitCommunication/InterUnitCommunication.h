
// Own protocol for controlling the remote pump and getting response back.
class InterUnitCommunication
{
  public:
    InterUnitCommunication()
      : m_stage(0)
    {
    }

    //CRC-8 - based on the CRC8 formula by Dallas/Maxim
    //code released under the terms of the GNU GPL 3.0 license
    static void AddCRC8(byte& crc, const byte* data, byte len) {
      while (len--) {
        byte extract = *data++;
        for (byte tempI = 8; tempI; tempI--) {
          byte sum = (crc ^ extract) & 0x01;
          crc >>= 1;
          if (sum) {
            crc ^= 0x8C;
          }
          extract >>= 1;
        }
      }
    }

    static byte CalcCRC(int temperature, bool pumpon, bool pumpforced)
    {
      byte crc = 0;
      AddCRC8(crc, (byte*)&temperature, sizeof(temperature));
      AddCRC8(crc, (byte*)&pumpon, sizeof(pumpon));
      AddCRC8(crc, (byte*)&pumpforced, sizeof(pumpforced));
      return crc;
    }

    static void Send(int temperature, bool pumpon, bool pumpforced)
    {
      Serial.print(F("@"));       // stage 0
      Serial.print(temperature);  // stage 1
      Serial.print(',');
      Serial.print(pumpon);       // stage 2
      Serial.print(',');
      Serial.print(pumpforced);   // stage 3
      Serial.print(',');
      Serial.print(CalcCRC(temperature, pumpon, pumpforced)); // stage 4
      Serial.print('\n');
    }

    bool Read()
    {
      while (Serial.available() > 0)
      {
        byte rec = Serial.read();
        if ( rec == '@')  // Not only at stage == 0, but always trigger on a new package.
        {
          m_stage = 1;
          m_temperature = 0;
          m_pumpOn = false;
          m_pumpForcedOn = false;
          continue;
        }
        if ( m_stage > 0 && rec == ',')
        {
          ++m_stage;
          continue;
        }
        switch(m_stage)
        {
          case 1:
            m_temperature *= 10;
            m_temperature += rec - '0';
            break;
          case 2:
            m_pumpOn = rec == '1';
            break;
          case 3:
            m_pumpForcedOn = rec == '1';
            break;
          case 4:
            if ( rec == '\n')
            {
              m_stage = 0;
              if ( CalcCRC(m_temperature, m_pumpOn, m_pumpForcedOn) == m_crc ) return true;
            }
            else
            {
              m_crc *= 10;
              m_crc += rec - '0';
            }
            break;
          default:
            m_stage = 0;
            break;
        }
      }
      return false;
    }

    int   m_temperature;    // The temperature in tenths of Celcius (transmitted as half degrees possitive only)
    bool  m_pumpOn;         // Is / should the pump (be) on.
    bool  m_pumpForcedOn;   // Is the pump forcibly on (because it hasn't run for x hours). Only from pump to controller.
    byte  m_crc;
    byte  m_stage;
};
