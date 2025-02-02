#include <ESP8266WiFi.h> 
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <stdio.h>
const char* ssid = "OpenWrt"; //Подключается к точке доступа OpenWrt 
char strok[30];
char buf[30];
long sec;
int ss;
IPAddress ip(192,168,1,111);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0); 
ESP8266WebServer server(8080);
OneWire ds(0); // Датчик температуры DS18B20 на GPIO - 0
const int led1 = 1; // GPIO - 1 LED R
const int led2 = 2; // GPIO - 2 LED W
void temper() {
byte i;
byte present = 0;
byte data[12];
byte addr[8];
float celsius;
if ( !ds.search(addr)) {
ds.reset_search();
delay(250);
return;
}
OneWire::crc8(addr, 7);
ds.reset();
ds.select(addr);
ds.write(0x44, 1); // start conversion, with parasite power on at the end 
delay(800); // maybe 750ms is enough, maybe not
present = ds.reset();
ds.select(addr); 
ds.write(0xBE); // Read Scratchpad
for ( i = 0; i < 9; i++) { // we need 9 bytes
data[i] = ds.read();
}
// Convert the data to actual temperature
// because the result is a 16 bit signed integer, it should
// be stored to an "int16_t" type, which is always 16 bits
// even when compiled on a 32 bit processor.
int16_t raw = (data[1] << 8) | data[0]; 
byte cfg = (data[4] & 0x60);
// at lower res, the low bits are undefined, so let's zero them
if (cfg == 0x00) raw = raw & ~7; // 9 bit resolution, 93.75 ms
else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
//// default is 12 bit resolution, 750 ms conversion time 
celsius = (float)raw / 16.0;
dtostrf(celsius,5,2,buf);
sprintf(strok,"<h1>Temperatura = %s Celsius</h1>",buf); 
}
void tem(){
server.send(200, "text/html",strok);
} 
void whi_on() {
digitalWrite(led2, 1);
server.send(200, "text/html","<h1>whi on</h1>");
}
void whi_off() {
digitalWrite(led2, 0);
server.send(200, "text/html","<h1>whi of</h1>");
}
void red_on() {
digitalWrite(led1, 0);
server.send(200, "text/html","<h1>red on</h1>");
}
void red_off() {
digitalWrite(led1, 1);
server.send(200, "text/html","<h1>red of</h1>");
}
void whi_sta(){
if (digitalRead(led1)==0 && digitalRead(led2)==1) server.send(200, "text/html","<h1>Status: RedOn WhiOn</h1>");
if (digitalRead(led1)==1 && digitalRead(led2)==1) server.send(200, "text/html","<h1>Status: RedOf WhiOn</h1>");
if (digitalRead(led1)==0 && digitalRead(led2)==0) server.send(200, "text/html","<h1>Status: RedOn WhiOf</h1>");
if (digitalRead(led1)==1 && digitalRead(led2)==0) server.send(200, "text/html","<h1>Status: RedOf WhiOf</h1>");
}
void root(){
server.send(200, "text/html","<h1>This is web-server on Wifi-esp-01.<br>He controls two led and temperature.<br>Command:/whion /whioff /redon /redoff /temp /status</h1>"); 
}
void setup(void){
pinMode(led1, OUTPUT);
pinMode(led2, OUTPUT);
digitalWrite(led1, 1);
digitalWrite(led2, 0);
WiFi.config(ip,gateway,subnet);
WiFi.begin(ssid);
// Wait for connection
while (WiFi.status() != WL_CONNECTED) {
delay(500);
}
temper();
server.on("/temp", tem);
server.on("/whion", whi_on);
server.on("/whioff", whi_off);
server.on("/redon", red_on);
server.on("/redoff", red_off);
server.on("/status", whi_sta);
server.on("/", root); 
server.begin();
// Serial.println("HTTP server started");
} 
void loop(void){ 
sec=millis()/1000;
ss=sec%60; // second
if(ss==0 || ss==30 ) {temper(); 
}
server.handleClient(); 
}
