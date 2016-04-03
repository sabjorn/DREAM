Notes:
To Do: draw diagram of dev-board.

Jan 7/16
* Setup another board on the network to display via NeoPixel LEDs. Originally hard coded the number of LEDs on the board to be 100 BUT the latency for updating that many seems to be too much.
** Possible soultion is to have the total number of LEDs be larger than necessary (100~) and have an endpoint (i.e. */length*) to adjust the number of pixels updated by */pix*.

Jan 6/16
* peak current ussage: 
** LEDs off (transmitting): 97mA~
** LEDs on full white: 150mA~

Jan 4/16
- need to come up with some way of setting IP address OR give HOSTNAME. (mDNS and DNS-SD responder (ESP8266mDNS library)?)
- both a Max/MSP and a Python object (and example code) need to be made to automatically deal with setting up a complex number of these things (e.g. Python object for each Node and Python object showing attaching several and doing some mapping)
Dec 22/15
-Successfully implemented Gyroscope OSC out and input to control NeoPixel LEDs. Example code is in [ESP8266_OSCExample.ino](https://github.com/sabjorn/dreamIO/tree/master/code/ESP8266_OSCExample/ESP8266_OSCExample)