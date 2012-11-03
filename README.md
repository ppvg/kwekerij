**WARNING: This is a work in progress.**

The goal of this *Arduino sketch* is to provide a greenhouse temperature alarm system. We're using a serial GSM module (using SoftwareSerial and the AT protocol) and Dallas OneWire temperature sensors (type DS18B20).

Goals
-----

* When the temperature becomes to low, the device sends out an SMS (text message) to a predefined phone number.
* When an SMS is sent to the device, it responds to the phone number of origin with a list of the current temperatures.
* When a sensor is removed or malfunctions, an alart is sent to the predefined phone number.
* Using three buttons, the user should be able to configure the threshold temperature for each zone/sensor separately; one button for "next sensor", one for + and one for - (in 0.1°C increments).

Current functionality
---------------------

The current code is a bit limited:

* Temperature polling (but not checking).
* Working ~~threshold control and~~ LCD display.
* Predefined sensor device addresses.

The next step is to implement threshold configuration and SMS functionality.

Compiling / uploading
---------------------

To get started:

* Copy `config.h.example` to `config.h` and edit it to fit your configuration.
* Download the [OneWire library][1] and copy it to where your Arduino setup will find it.

  [1]: http://www.pjrc.com/teensy/td_libs_OneWire.html "OneWire Arduino Library"

You should then be able to compile and upload the sketch using **Arduino IDE 1.0** (we haven't tested in earlier versions). In addition, you can use `gcc-avr` and `make` to compile and/or upload the sketch.

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
