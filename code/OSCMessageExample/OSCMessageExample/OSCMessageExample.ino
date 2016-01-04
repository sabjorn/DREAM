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

#include "wifiCred.h" //used to store SSID and PASS

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

/*Defines for changing compile time behaviour*/
//#define ACCESSPOINT //comment out to make local access point
//===========================================================================//

/*WIFI Credentials*/
// #define SSID "" //your network SSID (name) 
// #define PASS "password" //your network password
//===========================================================================//

#define BLINKPIN 14 //blinking LED pin

int status = 0;     // the Wifi radio's status

/*Static IP*/
//Currently not used
IPAddress ip(192, 168, 0, 22);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
//===========================================================================//

//set IP address of receiving system
#ifdef ACCESSPOINT
  IPAddress outIp(192,168,4,2); //for use as 'access point'
#else
  //IPAddress outIp(192,168, 0, 13); //set IP of 
  IPAddress outIp(192, 168, 1, 124);
#endif
//===========================================================================//

/*port numbers*/
WiFiUDP Udp; //make ESP8266 Udp instance
const unsigned int inPort = 8888;
const unsigned int outPort = 9999;
//===========================================================================//

/*UDP Scheduler*/
long old_time, curret_time = 0;
long delay_time = 10; //interval between UDP sends *NEEDED! Will crash otherwise
uint8_t old_val = 0;
//===========================================================================//

/*GPIO Control Callback*/
void LEDcontrol(OSCMessage &msg)
{
  if (msg.isInt(0))
  {
    digitalWrite(BLINKPIN, msg.getInt(0)); //LED is on LOW
  }
}
//===========================================================================//

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin(12, 13);
      int TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz). Comment this line if having compilation difficulties with TWBR.
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif  
  pinMode(BLINKPIN, OUTPUT);
  
  #ifdef ACCESSPOINT
    WiFi.mode(WIFI_AP); // make wireless access point
  #else
    // attempt to connect to Wifi network:
    while ( status != WL_CONNECTED) 
    { 
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(SSID);
      
      // Connect to WPA/WPA2 network: 
      WiFi.mode(WIFI_STA);   
      status = WiFi.begin(SSID, PASS);
      
      //WiFi.config(ip, gateway, subnet); //experimental
      // wait 10 seconds for connection:
      delay(1000);
    }
  #endif

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");

  Udp.begin(inPort); //input Udp stream

}

void loop() {
  curret_time = millis();

  // 
  while (curret_time - old_time > delay_time) 
  {
    // outgoing message to indicate network connectivity
    OSCMessage status("/time");
    status.add(int(micros()));
    Udp.beginPacket(outIp, outPort);
    status.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket(); // mark the end of the OSC Packet
    status.empty(); // free space occupied by message

    old_time = millis();
  }

  OSCMessage msgIn;
  int size;
 
  if( (size = Udp.parsePacket())>0)
  {
    while(size--)
      msgIn.fill(Udp.read());

    if(!msgIn.hasError())
      msgIn.dispatch("/led", LEDcontrol);
  }
}