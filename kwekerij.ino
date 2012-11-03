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
#include <LiquidCrystal.h>
#include <stdio.h>
#include "functions.h"
#include "config.h"

// All settings and constants are in "config.h".

OneWire ds(onewire);
LiquidCrystal lcd(lcdRS, lcdE, db4, db5, db6, db7);

int temps[maxSensors]; // in °C ×10
int threshold; // in °C ×10
byte numSensors;
unsigned long lastTempCheck = 0; // in millis()
unsigned long lastThresholdChange = 0; // in millis()
bool displayOn = false;

void setup(void) {
  pinMode(potentiometer, INPUT);
  digitalWrite(potentiometer, HIGH); // internal pull-up resistor
  lcd.begin(16, 2);
  Serial.begin(9600);
}

void loop(void) {
  //bool changed = getThreshold();
  updateDisplay(true);

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
  int val = 1023 - analogRead(potentiometer);
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
    lcd.clear();
    printTemp(temps[0]);
    printTemp(temps[1]);
    printTemp(temps[2]);
  } else if (displayTimedOut()) {
    displayOn = false;
    lcd.clear();
  }
}

void printTemp(int temp) {
  lcd.print(temp/10);  // prints the int part
  lcd.print("."); // print the decimal point
  unsigned int frac;
  if (temp >= 0)
      frac = temp - (temp/10)*10;
  else
      frac = (temp/10)*10 - temp;
  lcd.print(frac, DEC);
  lcd.print(" ");
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
