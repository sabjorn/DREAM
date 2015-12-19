/*Example code for OSC message send/recieve with ESP8266
Based on example code made available through CNMAT
Written by: Steven A. Bjornson | 19/12/15

Included is a Max/MSP patch for controlling a single LED on board.
Max patch uses CNMAT OSC externals*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include <OSCMessage.h>
#include <OSCBundle.h>

#include <Adafruit_NeoPixel.h>

#include "wifiCred.h" //used to store SSID and PASS

#define PIXPIN 15
#define NUMPIX 9
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIX, PIXPIN, NEO_GRB + NEO_KHZ800);

#define BLINKPIN 14 //blinking LED pin

//#define BUNDLE //comment out to use Message example
// #define SSID "" //your network SSID (name) 
// #define PASS "password" //your network password

int status = 0;     // the Wifi radio's status

IPAddress ip(192, 168, 0, 22);
//IPAddress outIp(192, 168, 1, 101); //could be used to hardcode IP of ESP
IPAddress outIp(192,168, 0, 13);

//port numbers
const unsigned int inPort = 8888;
const unsigned int outPort = 9999;

WiFiUDP Udp; //make ESP8266 Udp instance

void LEDcontrol(OSCMessage &msg)
{
  if (msg.isInt(0))
  {
    digitalWrite(BLINKPIN, !msg.getInt(0)); //LED is on LOW
  }
}

void pix(OSCMessage &msg)
{
  uint8_t r, g, b = 0;
  if (msg.isInt(0))
    r = msg.getInt(0);
  if (msg.isInt(1))
    g = msg.getInt(1);
  if (msg.isInt(2))
    b = msg.getInt(2);
  
  for (uint16_t i = 0; i < NUMPIX; i++)
    strip.setPixelColor(i, r, g, b);
  strip.show();
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  pinMode(BLINKPIN, OUTPUT);
 
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(SSID);
    // Connect to WPA/WPA2 network: 
    WiFi.mode(WIFI_STA);   
    status = WiFi.begin(SSID, PASS);
    // wait 10 seconds for connection:
    delay(1000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");

  Udp.begin(inPort); //input Udp stream

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

}

void loop() {
  // outgoing message to indicate network connectivity
  OSCMessage status("/time");
  status.add(int(micros()));
  
  Udp.beginPacket(outIp, outPort);
  status.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  status.empty(); // free space occupied by message

  // bundle or single message select
  #ifdef BUNDLE
  //Bundle example
  OSCBundle bundleIn;
  int size;
 
   if( (size = Udp.parsePacket())>0)
   {
     while(size--)
       bundleIn.fill(Udp.read());

      if(!bundleIn.hasError())
      {
        bundleIn.dispatch("/led", LEDcontrol);
        bundleIn.dispatch("/pix", pix);
      }
   }

  #else
  // single message example
  OSCMessage msgIn;
  int size;
 
  if( (size = Udp.parsePacket())>0)
  {
   while(size--)
     msgIn.fill(Udp.read());

    if(!msgIn.hasError())
      msgIn.dispatch("/led", LEDcontrol);
      msgIn.dispatch("/pix", pix);
  }
  #endif

  delay(20);
}