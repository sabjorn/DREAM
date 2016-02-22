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
#include <ESP8266mDNS.h>

#include <OSCMessage.h>
#include <OSCBundle.h>

#include <Adafruit_NeoPixel.h>
#include "wifiCred.h" //used to store SSID and PASS

#include <Math.h>

// Unique ID endpoint for each instance
#define ID "/2"

//MPU
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

MPU6050 mpu;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
//===========================================================================//

/*Defines for changing compile time behaviour*/
//#define BUNDLE //comment out to use Message example
//#define ACCESSPOINT //comment out to make local access point
//===========================================================================//

/*WIFI Credentials*/
// #define SSID "" //your network SSID (name) 
// #define PASS "password" //your network password
//===========================================================================//

/*Neopixels*/
#define PIXPIN 15
#define NUMPIX 8
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIX, PIXPIN, NEO_GRB + NEO_KHZ800);
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
  //IPAddress outIp(192,168, 0, 13); //set IP of SPECIFIC receiver
  //IPAddress outIp(192, 168, 1, 108);
  IPAddress outIp(255, 255, 255, 255); //broadcast UDP
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

/*Interrupt Detection Routine*/
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}
//===========================================================================//

/*GPIO Control Callback*/
void LEDcontrol(OSCMessage &msg)
{
  if (msg.isInt(0))
  {
    digitalWrite(BLINKPIN, !msg.getInt(0)); //LED is on LOW
  }
}
//===========================================================================//


/*Neopixel Control Callback*/
void pix(OSCMessage &msg)
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

float pi2float(float in){
  return ((in / M_PI) + 1) / 2;
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin(13, 12);
      int TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz). Comment this line if having compilation difficulties with TWBR.
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif

  mpu.initialize();
  devStatus = mpu.dmpInitialize();
  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
      // turn on the DMP, now that it's ready
      //Serial.println(F("Enabling DMP..."));
      mpu.setDMPEnabled(true);

      // enable Arduino interrupt detection
      //Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
      attachInterrupt(4, dmpDataReady, RISING);
      mpuIntStatus = mpu.getIntStatus();

      // set our DMP Ready flag so the main loop() function knows it's okay to use it
      //Serial.println(F("DMP ready! Waiting for first interrupt..."));
      dmpReady = true;

      // get expected DMP packet size for later comparison
      packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
      // ERROR!
      // 1 = initial memory load failed
      // 2 = DMP configuration updates failed
      // (if it's going to break, usually the code will be 1)
      // Serial.print(F("DMP Initialization failed (code "));
      // Serial.print(devStatus);
      // Serial.println(F(")"));
  }

  //===========================================================================//
  
  pinMode(BLINKPIN, OUTPUT);
  
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
  digitalWrite(BLINKPIN, HIGH); //LED turns off when WIFI connects

  Udp.begin(inPort); //input Udp stream

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

}

void loop() {
  // if programming failed, don't try to do anything
    if (!dmpReady) return;

    curret_time = millis();

    //Scheduler
    // wait for MPU interrupt or extra packet(s) available
    while (!mpuInterrupt && fifoCount < packetSize && curret_time - old_time > delay_time) {
      //declare a bundle
      OSCBundle bndl;
      bndl.add(ID"/time").add((int32_t)micros()); //time since active :: indicates a connection
      bndl.add(ID"/ypr").add(pi2float(ypr[0])).add(pi2float(ypr[1])).add(pi2float(ypr[2])); // yaw/pitch/roll

      Udp.beginPacket(outIp, outPort);
      bndl.send(Udp); // send the bytes to the SLIP stream
      Udp.endPacket(); // mark the end of the OSC Packet
      bndl.empty(); // empty the bundle to free room for a new one

      old_time = millis();
    }

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
        bundleIn.dispatch(ID"/led", LEDcontrol);
        bundleIn.dispatch(ID"/pix", pix);
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
    {
      Serial.println("DISPATCHING!");
      msgIn.dispatch(ID"/led", LEDcontrol);
      msgIn.dispatch(ID"/pix", pix);
    }
  }
  #endif

  // reset interrupt flag and get INT_STATUS byte
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  fifoCount = mpu.getFIFOCount();

  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
      // reset so we can continue cleanly
      mpu.resetFIFO();
      //Serial.println(F("FIFO overflow!"));

  // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);
    
    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;

    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
  }

}