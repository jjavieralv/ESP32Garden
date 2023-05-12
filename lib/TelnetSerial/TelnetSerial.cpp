#include <TelnetSerial.h>

TelnetSerial::TelnetSerial(int serial_baudrate,bool set_telnet,bool set_serial){
  set_telnet_enabled(set_telnet);
  set_serial_enabled(set_serial);
  Serial.begin(serial_baudrate);
  TelnetStream.begin();
}

TelnetSerial::~TelnetSerial(){
  return;
}

void TelnetSerial::set_telnet_enabled(bool set_telnet){
  telnet_enabled=set_telnet;
}
void TelnetSerial::set_serial_enabled(bool set_serial){
  serial_enabled=set_serial;
}

bool TelnetSerial::get_telnet_enabled(){
  return telnet_enabled;
}

bool TelnetSerial::get_serial_enabled(){
  return serial_enabled;
}

size_t TelnetSerial::write(uint8_t c){
  if(get_telnet_enabled())TelnetStream.write(c);
  if(get_serial_enabled())Serial.write(c);
}
size_t TelnetSerial::write(const uint8_t *buffer, size_t size){
  if(get_telnet_enabled())TelnetStream.write(buffer,size);
  if(get_serial_enabled())Serial.write(buffer,size);
}

size_t TelnetSerial::print(const __FlashStringHelper *ifsh){
  if(get_telnet_enabled())TelnetStream.print(ifsh);
  if(get_serial_enabled())Serial.print(ifsh);
}
size_t TelnetSerial::print(const String &s){
  if(get_telnet_enabled())TelnetStream.print(s);
  if(get_serial_enabled())Serial.print(s);
}
size_t TelnetSerial::print(const char str[]){
  if(get_telnet_enabled())TelnetStream.print(str);
  if(get_serial_enabled())Serial.print(str);
}
size_t TelnetSerial::print(char c){
  if(get_telnet_enabled())TelnetStream.print(c);
  if(get_serial_enabled())Serial.print(c);
}
size_t TelnetSerial::print(unsigned char b, int base){
  if(get_telnet_enabled())TelnetStream.print(b,base);
  if(get_serial_enabled())Serial.print(b,base);
}
size_t TelnetSerial::print(int n, int base){
  if(get_telnet_enabled())TelnetStream.print(n,base);
  if(get_serial_enabled())Serial.print(n,base);
}
size_t TelnetSerial::print(unsigned int n, int base){
  if(get_telnet_enabled())TelnetStream.print(n,base);
  if(get_serial_enabled())Serial.print(n,base);
}
size_t TelnetSerial::print(long n, int base){
  if(get_telnet_enabled())TelnetStream.print(n,base);
  if(get_serial_enabled())Serial.print(n,base);
}
size_t TelnetSerial::print(unsigned long n, int base){
  if(get_telnet_enabled())TelnetStream.print(n,base);
  if(get_serial_enabled())Serial.print(n,base);
}
size_t TelnetSerial::print(double n, int digits){
  if(get_telnet_enabled())TelnetStream.print(n,digits);
  if(get_serial_enabled())Serial.print(n,digits);
}
size_t TelnetSerial::print(const Printable& x){
  if(get_telnet_enabled())TelnetStream.print(x);
  if(get_serial_enabled())Serial.print(x);
}
size_t TelnetSerial::println(const __FlashStringHelper *ifsh){
  if(get_telnet_enabled())TelnetStream.println(ifsh);
  if(get_serial_enabled())Serial.println(ifsh);
}

size_t TelnetSerial::println(void){
  if(get_telnet_enabled())TelnetStream.println();
  if(get_serial_enabled())Serial.println();
}
size_t TelnetSerial::println(const String &s){
  if(get_telnet_enabled())TelnetStream.println(s);
  if(get_serial_enabled())Serial.println(s);
}
size_t TelnetSerial::println(const char c[]){
  if(get_telnet_enabled())TelnetStream.println(c);
  if(get_serial_enabled())Serial.println(c);
}
size_t TelnetSerial::println(char c){
  if(get_telnet_enabled())TelnetStream.println(c);
  if(get_serial_enabled())Serial.println(c);
}
size_t TelnetSerial::println(unsigned char b, int base){
  if(get_telnet_enabled())TelnetStream.println(b,base);
  if(get_serial_enabled())Serial.println(b,base);
}
size_t TelnetSerial::println(int num, int base){
  if(get_telnet_enabled())TelnetStream.println(num,base);
  if(get_serial_enabled())Serial.println(num,base);
}
size_t TelnetSerial::println(unsigned int num, int base){
  if(get_telnet_enabled())TelnetStream.println(num,base);
  if(get_serial_enabled())Serial.println(num,base);
}
size_t TelnetSerial::println(long num, int base){
  if(get_telnet_enabled())TelnetStream.println(num,base);
  if(get_serial_enabled())Serial.println(num,base);
}
size_t TelnetSerial::println(unsigned long num, int base){
  if(get_telnet_enabled())TelnetStream.println(num,base);
  if(get_serial_enabled())Serial.println(num,base);
}
size_t TelnetSerial::println(double num, int digits){
  if(get_telnet_enabled())TelnetStream.println(num,digits);
  if(get_serial_enabled())Serial.println(num,digits);
}
size_t TelnetSerial::println(const Printable& x){
  if(get_telnet_enabled())TelnetStream.println(x);
  if(get_serial_enabled())Serial.println(x);
}
