**WARNING: This is a work in progress.**

The goal of this *Arduino sketch* is to provide a greenhouse temperature alarm system. We're using a serial GSM module (using SoftwareSerial and the AT protocol) and Dallas OneWire temperature sensors (type DS18B20).

Goals
-----

* When the temperature becomes to low, the device sends out an SMS (text message) to a predefined phone number.
* When an SMS is sent to the device, it responds to the phone number of origin with a list of the current temperatures.
* When a sensor is removed or malfunctions, an alart is sent to the predefined phone number.

Current functionality
---------------------

The current code is a bit limited:

* Temperature polling (but not checking).
* Working threshold control and display (using a potetiometer and 3 LEDs).
* Display timeout.

The next step is to write code for the GSM module.

Compiling / uploading
---------------------

The sketch should work fine with Arduino IDE 1.0.

In addition, you can use `gcc-avr` and `make` to compile and/or upload the sketch.

* To compile: `make` or `make target`
* To upload: `make upload`
* Both: `make target upload`

The `Makefile` is currently configured for an Arduino Pro Mini 5v (with Atmega168). This can be configured by changing the `BOARD` variable in the `Makefile` to one of the names from Arduino's `boards.txt` (use Google), e.g. `"uno"`. **Note:** This is not needed if you're using the Arduino IDE.

License
-------

Copyright (C) 2012  Tijmen Leroi and Peter-Paul van Gemerden

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
