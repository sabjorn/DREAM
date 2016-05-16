/*Example code for OSC message send/recieve with ESP8266
Code is used to control NeoPixel LEDs using the Adafruit NeoPixel Library

Written by: Steven A. Bjornson | 08/01/16

Included is a Max/MSP patch for controlling the NeoPixels.
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

// Unique ID endpoint for each instance
#define ID "/1"

/*Neopixels*/
#define PIXPIN 15
#define NUMPIX 8
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIX, PIXPIN, NEO_GRB + NEO_KHZ800);
//===========================================================================//

int status = 0;     // the Wifi radio's status

//set IP address of receiving system
IPAddress outIp(255, 255, 255, 255); //broadcast UDP
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

/*Neopixel Control Callback*/
void leds(OSCMessage &msg)
{
  uint8_t n, r, g, b = 0;
  
  // change the colour of ALL pixels simultaneously
  if(msg.size() < 4)
  {
    if (msg.isInt(0))
      r = msg.getInt(0);
    if (msg.isInt(1))
      g = msg.getInt(1);
    if (msg.isInt(2))
      b = msg.getInt(2);
    
    for (uint16_t i = 0; i < NUMPIX; i++)
      strip.setPixelColor(i, r, g, b);
  }

  // change the colour of pixel 'n'.
  else
  {
    if (msg.isInt(0))
      n = msg.getInt(0);
    if (msg.isInt(1))
      r = msg.getInt(1);
    if (msg.isInt(2))
      g = msg.getInt(2);
    if (msg.isInt(3))
      b = msg.getInt(3);

    strip.setPixelColor(n, r, g, b);
  }
  strip.show();
}
//===========================================================================//

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  #ifdef ACCESSPOINT
    WiFi.mode(WIFI_AP); // make wireless access point
  #else
    // attempt to connect to Wifi network:
    while ( status != WL_CONNECTED) 
    { 
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(SSID);
      
      WiFi.hostname("testname");
      // Connect to WPA/WPA2 network: 
      WiFi.mode(WIFI_STA);   
      status = WiFi.begin(SSID, PASS);
      
      //WiFi.config(ip, gateway, subnet); //experimental
      // wait 1 seconds for connection:
      delay(1000);
    }
  #endif

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");

  Udp.begin(inPort); //input Udp stream

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}


void loop() {
	curret_time = millis();

	//Scheduler
	// wait for MPU interrupt or extra packet(s) available
	while (curret_time - old_time > delay_time) {
	  // outgoing message to indicate network connectivity
	  OSCMessage status(ID"/time");
	  status.add(int(micros()));
	  Udp.beginPacket(outIp, outPort);
	  status.send(Udp); // send the bytes to the SLIP stream
	  Udp.endPacket(); // mark the end of the OSC Packet
	  status.empty(); // free space occupied by message

	  old_time = millis();
	}

	  // single message example
	  OSCMessage msgIn;
	  int size;
 
	  if( (size = Udp.parsePacket())>0)
	  {
      while(size--)
	     msgIn.fill(Udp.read());

	    if(!msgIn.hasError())
	     msgIn.dispatch(ID"/leds", leds);
	  }
}
