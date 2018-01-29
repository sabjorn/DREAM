## D.R.E.A.M - *D*ata *R*ules *E*verything *A*round *M*e

## About
*dreamIO* is an ESP8266 based development board for designing interactive art applications. 

The board features MPU-6050 6-Axis IMU, TPS61200 Voltage Regulator, MCP73831 Lipo Charger, and 12x WS2812B RGB LEDs. 

The functionality of the board is made available through an OSC based API. Through this network interface, large, complex, and distributed interactive applications can be developed with little (to no) microcontroller programming. Instead, designers can use any language of their choice (any languague which supports UDP messaging) and any platform of their choice (Mac, Windows, Linux) to write the software which communicates with these devices. 

## Development Status
Early development stages, version 0.~

**Note**: The current PCB design in the git is broken from an attempt to fix 

### Arduino SDK info
Currently only version **2.0.0** of ESP8266 for Arduino is supported for this project. 

## External Libraries
* [ESP8266 Arduino](https://github.com/esp8266/Arduino) - 2.0.0
* [I2CDev](https://github.com/jrowberg/i2cdevlib) for the MPU-6050
* [OSC](https://github.com/CNMAT/OSC) 
* [Adafruit NeoPixels](https://github.com/adafruit/Adafruit_NeoPixel) - 1.0.6 **Use the Arduino Package Manager version**

## Future
The addition of a haptics motor is likely.