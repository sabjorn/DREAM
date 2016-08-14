##D.R.E.A.M - *D*ata *R*ules *E*verything *A*round *M*e

##About
*dreamIO* is an ESP8266 based development board for designing interactive art applications. 

The board features MPU-6050 6-Axis IMU, TPS61200 Voltage Regulator, MCP73831 Lipo Charger, and 12x WS2812B RGB LEDs. 

The functionality of the board is made available through an OSC based API. Through this network interface, large, complex, and distributed interactive applications can be developed with little (to no) microcontroller programming. Instead, designers can use any language of their choice (any languague which supports UDP messaging) and any platform of their choice (Mac, Windows, Linux) to write the software which communicates with these devices. 

Along with the raw sensor data from the onboard ICs, the API exposes pre-processed 'features' for use in applications. For example, there is an OSC endpoint which tells the user if the board is in motion. The goal of these features is to reduce the cognitive overhead of development so that designers can focus on making interesting things.

The firmware is open and easily accessable and modification is encurouaged (but not necessary).

##Development Status
Early development stages, version 0.~

**Note**: The current PCB design in the git is broken from an attempt to fix 

##External Libraries
* [ESP8266 Arduino](https://github.com/esp8266/Arduino) for the ESP8266
* [I2CDev](https://github.com/jrowberg/i2cdevlib) for the MPU-6050
* [OSC](https://github.com/CNMAT/OSC) slightly modified to get it to compile for ESP8266
* [Adafruit NeoPixels](https://github.com/adafruit/Adafruit_NeoPixel)

##Future
* More sensors
* More actuators
* More features