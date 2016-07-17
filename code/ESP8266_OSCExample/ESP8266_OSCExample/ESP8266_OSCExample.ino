/*Example code for OSC message send/recieve with ESP8266
Based on example code made available through CNMAT
Written by: Steven A. Bjornson | 19/12/15

Included is a Max/MSP patch for controlling a single LED on board.
Max patch uses CNMAT OSC externals*/
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
// #include <WiFiClient.h>
// #include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

#include <OSCMessage.h>
#include <OSCBundle.h>

#include <Adafruit_NeoPixel.h>
#include "wifiCred.h" //used to store SSID and PASS

#include <Math.h>

/*Voltage Measurement*/
// Voltage divider on ADC allows for a measurement of battery voltage.
// maybe these don't need to be preprocessor
#define V_RES float(1./256.) //the number of steps per volt
#define R1 float(2000000.)
#define R2 float(330000.)
#define V_SCALE float(R2 / (R1 + R2))
#define V_MIN float(2.6) //the undervoltage shutoff of voltage regulator
#define V_MAX float(4.2) //the theoretical maximum voltage of LiPo
#define SCALED_V_MIN float(V_SCALE * V_MIN / V_RES)
#define SCALED_V_MAX float(V_SCALE * V_MAX / V_RES)

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

//MPU soft-wire
#define SDA_PIN 13
#define SCL_PIN 12

/*Neopixels*/
#define PIX_PIN 15
#define NUMPIX 12
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIX, PIX_PIN, NEO_GRB + NEO_KHZ800);
//===========================================================================//

int status = 0;     // the Wifi radio's status

IPAddress outIp(255, 255, 255, 255); //broadcast UDP
//===========================================================================//

/*UDP*/
WiFiUDP Udp; //make ESP8266 Udp instance
const unsigned int inPort = 8888;
const unsigned int outPort = 9999;
//===========================================================================//

/*UDP Scheduler*/
long old_time, curret_time = 0;
long delay_time = 25; //interval between UDP sends *NEEDED! Will crash otherwise
uint8_t old_val = 0;
//===========================================================================//

/*OTA*/
int prog = 0; //keeps track of download prograss for OTA to be displayed on WS2812s

/*Interrupt Detection Routine*/
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}
//===========================================================================//

/*Neopixel Control Callback*/
void leds(OSCMessage &msg)
{
  uint8_t n, r, g, b = 0;
  
  // change the colour of ALL pixelsels simultaneously
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

//Callback to change delay_time
void update_interval(OSCMessage &msg)
{
  if(msg.isInt(0))
    delay_time = long(msg.getInt(0));
  if(msg.isFloat(0))
    delay_time = long(msg.getFloat(0));

  if (delay_time < 10)
    delay_time = 10;
}

//callback to restart ESP
void reset(OSCMessage &msg)
{
  ESP.reset();
}

float max_ypr[3] = {-10, -10, -10};
float min_ypr[3] = {10, 10, 10};


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  //=========================================================================//

  /*Neopixels*/
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  //=========================================================================//

  /* join I2C bus */
  //(I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin(SDA_PIN, SCL_PIN);
      int TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz). Comment this line if having compilation difficulties with TWBR.
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif
  //=========================================================================//

  /*MPU6050*/
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
  
  uint8_t wifi_loading = 0;

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) 
  { 
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(SSID);
    
    // Connect to WPA/WPA2 network: 
    WiFi.mode(WIFI_STA);   
    status = WiFi.begin(SSID, PASS);
    
    //LEDs cycle through R/G/B while connecting
    for (uint16_t i = 0; i < NUMPIX; ++i)
    {
      switch(wifi_loading % 3){
        case 0:
          strip.setPixelColor(i, 255, 0, 0);
          break;
        case 1:
          strip.setPixelColor(i, 0, 255, 0);
          break;
        case 2:
          strip.setPixelColor(i, 0, 0, 255);
          break;
      }
    }
    strip.show();
    wifi_loading++;

    // wait 1 seconds for connection:
    delay(1000);
  }
  
  //clear
  for (uint16_t i = 0; i < NUMPIX; i++)
    strip.setPixelColor(i, 0);
  strip.show();

  // you're connected now, so print out the data:
  // Serial.println("You're connected to the network");
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());
  //=========================================================================//

  /*Setup OSC*/
  Udp.begin(inPort); //input Udp stream

  /*Over the Air Updates*/  
  //Progress Bar
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("End");

    //LEDs turn blue when update complete
    for (uint16_t i = 0; i < NUMPIX; i++)
      strip.setPixelColor(i, 0, 0, 255);
    strip.show();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\n", (progress / (total / 100)));

    if (prog != progress/float(total) * (NUMPIX+1))
    {
      for (uint16_t i = 0; i < NUMPIX; ++i)
      {
        if(i < prog)
          strip.setPixelColor(i, 0, 255, 0);
        else
          strip.setPixelColor(i, 0);
      }
      strip.show();

      prog = progress/float(total) * NUMPIX;
    }

  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");

    //LEDs turn red if failed to update
    for (uint16_t i = 0; i < NUMPIX; i++)
      strip.setPixelColor(i, 255, 0, 0);
    strip.show();
  });

  //Start OTA Service
  ArduinoOTA.begin();
  //=========================================================================//

}

void loop() {
  ArduinoOTA.handle(); //check OTA
  
  if (!dmpReady) return; // if programming failed, don't try to do anything

  /*Scheduler*/
  //possible make object in future  
  curret_time = millis(); 
  
  // wait for MPU interrupt or extra packet(s) available
  while (!mpuInterrupt && fifoCount < packetSize && curret_time - old_time > delay_time) {
    /*OSC Out*/
    //declare a bundle
    OSCBundle bndl;
    bndl.add("/time").add((int32_t)millis()); //time since active :: indicates a connection
    bndl.add("/ypr").add(ypr[0]).add(ypr[1]).add(ypr[2]); // yaw/pitch/roll
    bndl.add("/batt").add(float((analogRead(A0) >> 2)-SCALED_V_MIN)/(SCALED_V_MAX - SCALED_V_MIN)); //battery voltage [0,1]
    bndl.add("/debug").add(min_ypr[0]).add(max_ypr[0]).add(min_ypr[1]).add(max_ypr[1]).add(min_ypr[2]).add(max_ypr[2]);

    Udp.beginPacket(outIp, outPort);
    bndl.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket(); // mark the end of the OSC Packet
    bndl.empty(); // empty the bundle to free room for a new one
    //=======================================================================//

    old_time = millis();
  }
  //=========================================================================//

  /*OSC In*/
  #ifdef BUNDLE
    //Bundle example
    OSCBundle OSCin;
  #else
    // single message example
    OSCMessage OSCin;
  #endif
  
  int size;
 
  if( (size = Udp.parsePacket())>0)
  {
    while(size--)
     OSCin.fill(Udp.read());

    if(!OSCin.hasError())
    {
      OSCin.dispatch("/leds", leds);
      OSCin.dispatch("/update", update_interval);
      OSCin.dispatch("/reset", reset);
    }
  }
  //=========================================================================//

  /*MPU6050 Processes*/
  mpuInterrupt = false; // reset interrupt flag and get INT_STATUS byte
  mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  fifoCount = mpu.getFIFOCount();

  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
      // reset so we can continue cleanly
      mpu.resetFIFO();
      //Serial.println(F("FIFO overflow!"));
  } 
  // otherwise, check for DMP data ready interrupt (this should happen frequently)
  else if (mpuIntStatus & 0x02) {
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

    for (int i = 0; i < 3; ++i){
      if (i == 0)
        ypr[i] = ((ypr[i] / M_PI) + 1) / 2;
      else
        ypr[i] = ((ypr[i] / (M_PI/2.)) + 1) / 2;
    }
  }
  //=========================================================================//
}