
/***************************************************************
 * String formatting class, compatible with the Print interface
 ***************************************************************/
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

