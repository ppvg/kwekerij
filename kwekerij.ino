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
#include <EEPROM.h>
#include <stdio.h>
#include "functions.h"
#include "config.h"

// All settings and constants are in "config.h".

OneWire ds(onewire);
LiquidCrystal lcd(lcdRS, lcdE, db4, db5, db6, db7);

int temps[numSensors]; // in °C ×10
byte nextTempToCheck = 0;
int thresholds[numSensors]; // in °C ×10
int selectedThreshold = -1;
unsigned long lastTempCheck = 0; // in millis()
unsigned long settingsTimeout= 0; // in millis()

void setup(void) {
  lcd.begin(16, 2);
  Serial.begin(9600);
  loadThresholds();
}

void loop(void) {
  if (tempCheckNeeded()) {
    unsigned long start = millis();
    readNextTemp();
    Serial.print("Temp lookup took ");
    Serial.print(millis() - start);
    Serial.println(" ms.");
    updateDisplay(true);
  }
  delay(10);
}

//---- Function definitions -----//

void loadThresholds() {
  for (byte i=0; i<numSensors; i++) {
    thresholds[i] = readThreshold(i);
  }
}

int readThreshold(byte i) {
  byte bytes[2] = {
    EEPROM.read(i*2),
    EEPROM.read(i*2 + 1)
  };
  return (bytes[1] << 8) | bytes[0];
}

void writeThreshold(byte i, int value) {
  EEPROM.write(i*2, value & 0xFF);
  EEPROM.write(i*2 + 1, (value >> 8) & 0xFF);
}

bool tempCheckNeeded() {
  return (millis() - lastTempCheck) > (tempCheckTimeout / numSensors);
}

void readNextTemp() {
  byte data[9];
  getRawData(sensorAddresses[nextTempToCheck], data);
  temps[nextTempToCheck] = dataToCelcius(data, 0);
  lastTempCheck = millis();
  Serial.print(nextTempToCheck);
  Serial.print(" is ");
  Serial.println(temps[nextTempToCheck]);
  if (nextTempToCheck >= numSensors - 1)
    nextTempToCheck = 0;
  else
    nextTempToCheck += 1;
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

void updateDisplay(bool changed) {
  if (changed) {
    lcd.clear();
    printTemp(temps[0]);
    printTemp(temps[1]);
    printTemp(temps[2]);
    lcd.setCursor(0, 1);
    printTemp(temps[3]);
    printTemp(temps[4]);
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
