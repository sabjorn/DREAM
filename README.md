##D.R.E.A.M - *D*ata *R*ules *E*verything *A*round *M*e

##About
*dreamIO* is a ESP8266 based development board with onboard MPU-6050 6-Axis IMU, TPS61200 Voltage Regulator, MCP73831 Lipo Charger, and 12x WS2812B RGB LEDs.

This project is devoted to creating a tool for interactive art. By linking together WIFI, LEDs, and Sensors, artists will be able to create complex interactive works without the overhead of hacking together existing interfaces or designing new hardware from scratch.

The firmware on the board uses a OSC stream based API for the creation of complex interactive applications in any language which supports OSC.

##Development Status
Early development stages, version 0.~
**Note**: The current PCB design in the git is broken from an attempt to fix 

##External Libraries
    * [ESP8266 Arduino](https://github.com/esp8266/Arduino) for the ESP8266
    * [I2CDev](https://github.com/jrowberg/i2cdevlib) for the MPU-6050
    * [OSC](https://github.com/CNMAT/OSC) slightly modified to get it to compile for ESP8266
    * [Adafruit NeoPixels](https://github.com/adafruit/Adafruit_NeoPixel)

##Future
The addition of a haptics motor is likely.