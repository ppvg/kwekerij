/*
* Copyright (C) 2012  Tijmen Leroi and Peter-Paul van Gemerden
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <OneWire.h>
#include "functions.h"
#include "config.h"

// All settings and constants are in "config.h".

OneWire ds(onewirePin);
int temps[maxSensors]; // in °C ×10
int threshold; // in °C ×10
byte numSensors;
unsigned long lastTempCheck = 0; // in millis()
unsigned long lastThresholdChange = 0; // in millis()
bool displayOn = false;

void setup(void) {
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
  pinMode(potPin, INPUT);
  digitalWrite(potPin, HIGH); // internal pull-up resistor
  Serial.begin(9600);
}

void loop(void) {
  bool changed = getThreshold();
  updateDisplay(changed);

  if (tempCheckNeeded()) {
    getTemps();
    printTemps();
  }

  delay(100);
}


//---- Function definitions -----//

bool tempCheckNeeded() {
  return millis() - lastTempCheck > tempCheckTimeout;
}

void getTemps() {
  byte addr[8];
  byte data[9];
  byte sensorType = 0; // Our sensors are all DS18B20 (see getSensorType)

  numSensors = 0;
  while (numSensors < maxSensors && ds.search(addr)) {
    if (OneWire::crc8(addr, 7) == addr[7]) {
      numSensors += 1;
      getRawData(addr, data);
      temps[numSensors-1] = dataToCelcius(data, sensorType);
    } else {
      Serial.println("CRC is not valid!");
    }
  }
  ds.reset_search();
  lastTempCheck = millis();
}

byte getRawData(byte addr[], byte data[]) {
  byte present = 0;
  ds.reset();
  ds.select(addr);
  ds.write(0x44); // "Start Conversion", without parasite power
  present = ds.reset(); // 1 if device asserted presence pulse
  ds.select(addr);
  ds.write(0xBE); // "Read Scratchpad"
  for (byte i=0; i<9; i++) {
    data[i] = ds.read();
  }
  return present;
}

int dataToCelcius(byte data[], byte sensorType) {
  unsigned int raw = (data[1] << 8) | data[0];
  if (sensorType) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) { // count remain gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60); // default, 12 bit res, 750 ms
    if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
  }
  return (raw * 10) / 16;
}

void printTemps() {
  Serial.print("Number of sensors: ");
  Serial.println(numSensors);
  for (byte i=0; i<numSensors; i++) {
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print((float)temps[i]/10);
    Serial.println(" °C");
  }
}

bool getThreshold() {
  int val = 1023 - analogRead(potPin);
  float newThreshold = ((float)val/1023) * winSize + tempWinLow;
  int diff = threshold - newThreshold;
  if (diff < -thresholdMinChange || diff > thresholdMinChange) {
    Serial.print("New threshold: ");
    Serial.print(newThreshold/10);
    Serial.println(" °C");
    threshold = (int)newThreshold;
    return true;
  }
  return false;
}

void updateDisplay(bool changed) {
  if (!changed && !displayOn) { return; }

  if (changed) {
    int led1 = 0, led2 = 0, led3 = 0;
    lastThresholdChange = millis();
    displayOn = true;
    float factor = (float)(threshold-tempWinLow) / winSize;
    Serial.println(factor);
    if (factor < 0.5) {
      int value = 255 * factor * 2;
      Serial.print("<0.5: ");
      Serial.println(value);
      led1 = 255 - value;
      led2 = value;
    } else {
      int value = 255 * (1 - factor) * 2;
      Serial.print(">0.5: ");
      Serial.println(value);
      led2 = value;
      led3 = 255 - value;
    }
    analogWrite(led1Pin, led1);
    analogWrite(led2Pin, led2);
    analogWrite(led3Pin, led3);
  } else if (displayTimedOut()) {
    displayOn = false;
    analogWrite(led1Pin, 0);
    analogWrite(led2Pin, 0);
    analogWrite(led3Pin, 0);
  }
}

bool displayTimedOut() {
  return millis() - lastThresholdChange > displayTimeout;
}


//---------- Unused functions ---------//

byte getSensorType(byte addr[]) {
  switch (addr[0]) {
    case 0x10: // DS18S20 (old)
      return 1;
    case 0x28: // DS18B20
    case 0x22: // DS1822
      return 0;
    default: // Not a DS18x20 family device.
      return -1;
  }
}

// vim:softtabstop=2:shiftwidth=2:expandtab:
