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
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <stdio.h>
#include "Button.h"
#include "kwekerij.h"
#include "config.h"

// All settings and constants are in "config.h".

OneWire        onewire(oneWirePin);
LiquidCrystal  lcd(lcdRS, lcdE, db4, db5, db6, db7);
SoftwareSerial gsmSerial(gsmRX, gsmTX);
Button         btnNext(btnNextPin);
Button         btnUp(btnUpPin);
Button         btnDown(btnDownPin);

int temps[numSensors]; // in °C ×10
int thresholds[numSensors]; // in °C ×10
byte nextTempToCheck = 0;
byte selectedThreshold = numSensors; // numSensors == "overview"
unsigned long lastTempCheck = 0;
unsigned long lastButtonPress = millis();
unsigned long lastAlarmSMS = 0;
bool displayActive = true;
bool alarmActive = false;

void setup(void) {
  pinMode(backlight, OUTPUT);
  digitalWrite(backlight, HIGH);
  lcd.begin(16, 2);
  Serial.begin(9600);
  gsmSerial.begin(9600);
  loadThresholds();
  warmUp();
}

void loop(void) {
  if (displayActive) {
    if (displayTimedOut()) {
      deactivateDisplay();
    } else {
      handleButtonPress();
    }
  } else if (buttonPressed()) {
    activateDisplay();
  }

  if (tempCheckNeeded()) {
    readNextTemp();
    checkTemps();
    updateDisplay();
  }
  delay(20);
}

//---- Function definitions -----//

void warmUp() {
  Serial.println("Bezig met opwarmen...");
  lcd.clear();
  lcd.print("   Bezig met    ");
  lcd.setCursor(0, 1);
  lcd.print("   opwarmen...  ");
  initGsm();
  for (byte i=0; i<3*numSensors; i++) {
    readNextTemp();
  }
  if (!gsmOk()) {
    haltWithGsmError();
  }
  Serial.println("Opwarmen voltooid.");
  lastButtonPress = millis();
}

void initGsm() {
  delay(2000);
  gsmSerial.print("AT+CMGF=1\r"); // Set text mode.
  delay(1000);
  if (!gsmOk()) {
    haltWithGsmError();
  }
  gsmSerial.print("AT+CPIN=0000\r");  // Enter pin, hardcoded
  delay(1000);
}

bool gsmOk() {
  /*gsmSkipLine();*/
  char line[100];
  gsmReadLine(line);
  return strcmp(line, "OK\r\n") == 0;
}

void haltWithGsmError() {
  while (!gsmOk()) {
    Serial.println("Opstarten GSM mislukt!");
    lcd.clear();
    lcd.print(" Opstarten GSM  ");
    lcd.setCursor(0, 1);
    lcd.print("    mislukt!    ");
    delay(5000);
  }
}

void gsmSkipLine() {
  for (byte i=0; i<255; i++) {
    while (!gsmSerial.available()) delay(10);
    if (gsmSerial.read()== '\n') break;
  }
}

void gsmReadLine(char *line) {
  byte i=0;
  for (; i<100; i++) {
    while (!gsmSerial.available()) delay(10);
    line[i] = gsmSerial.read();
    if (line[i] == '\n') break;
  }
  i++;
  line[i]=0;
}


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

bool displayTimedOut() {
  return displayTimeout < (millis() - lastButtonPress);
}

void handleButtonPress() {
  bool displayNeedsUpdate = true;
  if (btnNext.isPressed()) {
    Serial.println("Next");
    selectedThreshold += 1;
    if (selectedThreshold > numSensors)
      selectedThreshold = 0;
    lastButtonPress = millis();

  } else if (btnUp.isPressed()) {
    Serial.println("Up");
    if (selectedThreshold < numSensors) {
      thresholds[selectedThreshold] += 1;
      writeThreshold(selectedThreshold, thresholds[selectedThreshold]);
    }
    lastButtonPress = millis();
  } else if (btnDown.isPressed()) {
    Serial.println("Down");
    if (selectedThreshold < numSensors) {
      thresholds[selectedThreshold] -= 1;
      writeThreshold(selectedThreshold, thresholds[selectedThreshold]);
    }
    lastButtonPress = millis();
  } else {
    displayNeedsUpdate = false;
  }
  if (displayNeedsUpdate) {
    updateDisplay();
  }
}

bool buttonPressed() {
  return btnNext.isPressed() || btnUp.isPressed() || btnDown.isPressed();
}

void deactivateDisplay() {
  selectedThreshold = numSensors;
  displayActive = false;
  updateDisplay();
}

void activateDisplay() {
  lastButtonPress = millis();
  displayActive = true;
  updateDisplay();
}

bool tempCheckNeeded() {
  return (millis() - lastTempCheck) > (tempCheckTimeout / numSensors);
}

void readNextTemp() {
  byte data[9];
  byte present = getRawData(sensorAddresses[nextTempToCheck], data);
  if (present) {
    temps[nextTempToCheck] = dataToCelcius(data, 0);
  } else {
    temps[nextTempToCheck] = -42;
  }
  lastTempCheck = millis();
  if (nextTempToCheck >= numSensors - 1)
    nextTempToCheck = 0;
  else
    nextTempToCheck += 1;
}

byte getRawData(byte addr[], byte data[]) {
  byte present = 0;
  onewire.reset();
  onewire.select(addr);
  onewire.write(0x44); // "Start Conversion", without parasite power
  present = onewire.reset(); // 1 if device asserted presence pulse
  onewire.select(addr);
  onewire.write(0xBE); // "Read Scratchpad"
  for (byte i=0; i<9; i++) {
    data[i] = onewire.read();
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

void checkTemps() {
  bool alarm = false;
  for (byte i=0; i<numSensors; i++) {
    if (temps[i] < thresholds[i]) {
      alarm = true;
    }
  }
  alarmActive = alarm;
  if (alarm) {
    Serial.println("--- ALARM ---");
    if (alarmSMSNeeded()) {
      sendAlarmSMS();
    }
  }
}

bool alarmSMSNeeded() {
  if (lastAlarmSMS == 0)
    return true;
  return alarmTimeout < (millis() - lastAlarmSMS);
}

void sendAlarmSMS() {
  lastAlarmSMS = millis();
  Serial.println("=== SENDING ALARM SMS ===");
  delay(100);
  gsmSerial.print("AT+CMGS=");
  gsmSerial.print(alarmNumber);
  gsmSerial.print("\r");
  delay(100);
  gsmSerial.print("Alarm! Temperatuur te laag.\n");
  for (byte i=0; i<numSensors; i++) {
    gsmSerial.print("\n");
    smsTemp(temps[i]);
  }
  delay(10);
  gsmSerial.print("\x1A"); //Send it ascii SUB
  delay(100);
}

void updateDisplay() {
  lcd.clear();
  if (displayActive) {
    digitalWrite(backlight, HIGH);
    if (selectedThreshold < numSensors) {
      lcd.print("Grenswaarde");
      lcd.setCursor(0, 1);
      lcd.print("Kas ");
      lcd.print(selectedThreshold+1);
      lcd.print(": ");
      printTemp(thresholds[selectedThreshold]);
      lcd.print("C");
    } else if (selectedThreshold == numSensors) {
      printTemp(temps[0]);
      printTemp(temps[1]);
      printTemp(temps[2]);
      lcd.setCursor(0, 1);
      printTemp(temps[3]);
      printTemp(temps[4]);
    } else {
      lcd.print("Ewut?");
    }
  } else {
    digitalWrite(backlight, LOW);
  }
}

void printTemp(int temp) {
  if (temp < 0 && temp > -10) {
    lcd.print("-");
  }
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

void smsTemp(int temp) {
  if (temp < 0 && temp > -10) {
    gsmSerial.print("-");
  }
  gsmSerial.print(temp/10);  // prints the int part
  gsmSerial.print("."); // print the decimal point
  unsigned int frac;
  if (temp >= 0)
      frac = temp - (temp/10)*10;
  else
      frac = (temp/10)*10 - temp;
  gsmSerial.print(frac, DEC);
  gsmSerial.print(" ");
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
