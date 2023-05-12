#ifndef TELNETSERIAL_H
#define TELNETSERIAL_H
#include "Arduino.h"
#include <TelnetStream.h>


class TelnetSerial{
  private:
    bool telnet_enabled=true;
    bool serial_enabled=true;

  public:
    TelnetSerial(int serial_baudrate,bool tn_enabled,bool seri_enabled);
    ~TelnetSerial();
    void set_telnet_enabled(bool set_telnet);
    void set_serial_enabled(bool set_serial);
    bool get_telnet_enabled();
    bool get_serial_enabled();

    size_t write(uint8_t);
    size_t write(const uint8_t *buffer, size_t size);
    size_t print(const __FlashStringHelper *);
    size_t print(const String &);
    size_t print(const char[]);
    size_t print(char);
    size_t print(unsigned char, int = DEC);
    size_t print(int, int = DEC);
    size_t print(unsigned int, int = DEC);
    size_t print(long, int = DEC);
    size_t print(unsigned long, int = DEC);
    size_t print(double, int = 2);
    size_t print(const Printable&);

    size_t println(const __FlashStringHelper *);
    size_t println(const String &s);
    size_t println(const char[]);
    size_t println(char);
    size_t println(unsigned char, int = DEC);
    size_t println(int, int = DEC);
    size_t println(unsigned int, int = DEC);
    size_t println(long, int = DEC);
    size_t println(unsigned long, int = DEC);
    size_t println(double, int = 2);
    size_t println(const Printable&);
    size_t println(void);


};



#endif