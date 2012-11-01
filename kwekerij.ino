#include <OneWire.h>

// Constants
#define onewirePin 10
#define ledLowPin  6
#define ledHighPin 9
#define potPin A0
#define maxSensors 10

// Function prototypes
void getTemps();
bool validCRC(byte addr[]);
float dataToCelcius(byte data[], byte sensor_type);
byte getSensorType(byte addr[]);


OneWire ds(onewirePin);
float temps[maxSensors];
byte numSensors;

void setup(void) {
  pinMode(ledLowPin, OUTPUT);
  pinMode(ledHighPin, OUTPUT);
  pinMode(potPin, INPUT);
  digitalWrite(potPin, HIGH); // internal pull-up resistor
  Serial.begin(9600);
}

void loop(void) {
  getTemps();
  Serial.print("Number of sensors: ");
  Serial.println(numSensors);
  for (byte i=0; i<numSensors; i++) {
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(temps[i]);
    Serial.println(" °C");
  }

  int val = analogRead(potPin);
  Serial.print("Potvalue is: ");
  Serial.print(val);
  Serial.println(".");
}


//---- Function definitions -----//

void getTemps() {
  byte addr[8];
  byte data[12];
  byte sensorType = 0; // Our sensors are all DS18B20 (see getSensorType)
  byte present = 0;

  numSensors = 0;

  while (numSensors < maxSensors && ds.search(addr)) {
    if (validCRC(addr)) {
      numSensors += 1;

      ds.reset();
      ds.select(addr);
      ds.write(0x44); // "Start Conversion", without parasite power

      present = ds.reset(); // returns 1 if device asserted presence pulse
      ds.select(addr);
      ds.write(0xBE); // "Read Scratchpad"

      for (byte i=0; i<9; i++) { // read all 9 bytes
        data[i] = ds.read();
      }
      temps[numSensors-1] = dataToCelcius(data, sensorType);
    } else {
      Serial.println("CRC is not valid!");
    }
  }
  ds.reset_search();
}

bool validCRC(byte addr[]) {
  return OneWire::crc8(addr, 7) == addr[7];
}

float dataToCelcius(byte data[], byte sensorType) {
  unsigned int raw = (data[1] << 8) | data[0];
  if (sensorType) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // count remain gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
    // default is 12 bit resolution, 750 ms conversion time
  }
  return (float)raw / 16.0;
}

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
