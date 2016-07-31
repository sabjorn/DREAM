This is set of example code for sending/recieving OSC Messages and Bundles on the ESP8266 using WIFI.

The example uses CNMAT's [OSC library](https://github.com/CNMAT/OSC) modified slighly (in `OSCData.h` and `.cpp`: `OSCData::OSCData(int i)` has been commented out; there was conflict between `int` and some other variable type).

In the code, an LED and NeoPixels can be controlled over OSC. A maxpatch has been included to facilitate this control.

Also, the ESP8266 streams it's `micros()` value to `/time`.