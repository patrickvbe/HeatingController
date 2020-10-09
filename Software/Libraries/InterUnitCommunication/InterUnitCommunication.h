
#define ReadDigit(var, digit) var *= 10; var += digit - '0';

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

    static byte CalcCRC(int temperature, bool pump_on, bool pump_forced)
    {
      byte crc = 0;
      AddCRC8(crc, (byte*)&temperature, sizeof(temperature));
      AddCRC8(crc, (byte*)&pump_on, sizeof(pump_on));
      AddCRC8(crc, (byte*)&pump_forced, sizeof(pump_forced));
      return crc;
    }

    static void Send(int temperature, bool pump_on, bool pump_forced)
    {
      Serial.print(F("<"));       // stage 0
      Serial.print(temperature);  // stage 1
      Serial.print(',');
      Serial.print(pump_on);       // stage 2
      Serial.print(',');
      Serial.print(pump_forced);   // stage 3
      Serial.print(',');
      Serial.print(CalcCRC(temperature, pump_on, pump_forced)); // stage 4
      Serial.print(">\n");
    }

    bool Read()
    {
      // No while, but if. Don't block here. We have state, so we will continue where we left of.
      if (Serial.available() > 0)
      {
        byte buffer[16];
        int count = Serial.readBytesUntil('\n', buffer, 16);
        for ( int pos=0; pos < count; pos++)
        {
          byte rec = buffer[pos];
          if ( rec == '<')  // Not only at stage == 0, but always trigger on a new package.
          {
            m_stage = 1;
            m_temperature = 0;
            m_pumpOn = false;
            m_pumpForcedOn = false;
            m_crc = 0;
            continue;
          }
          if ( m_stage > 0 && rec == ',')
          {
            ++m_stage;
            continue;
          }
          switch(m_stage)
          {
            case 1: ReadDigit(m_temperature, rec); break;
            case 2: m_pumpOn = rec == '1'; break;
            case 3: m_pumpForcedOn = rec == '1'; break;
            case 4:
              if ( rec == '>')
              {
                m_stage = 0;
                if ( CalcCRC(m_temperature, m_pumpOn, m_pumpForcedOn) == m_crc ) return true;
              }
              else ReadDigit(m_crc, rec);
              break;
            default: m_stage = 0; break;
          } // switch
        } // for
      } // while
      return false;
    }

    int   m_temperature;    // The temperature in tenths of Celsius (transmitted as half degrees positive only)
    bool  m_pumpOn;         // Is / should the pump (be) on.
    bool  m_pumpForcedOn;   // Is the pump forcibly on (because it has not run for x hours). Only from pump to controller.
    byte  m_crc;
    byte  m_stage;
};
