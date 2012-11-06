#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

// Function prototypes
void loadThresholds();
int readThreshold(byte i);
void writeThreshold(byte i, int value);
bool tempCheckNeeded();
void readNextTemp();
byte getRawData(byte addr[], byte data[]);
int dataToCelcius(byte data[], byte sensor_type);
bool getThreshold();
void updateDisplay();
void printTemp(int temp);
byte getSensorType(byte addr[]);

#endif

// vim:softtabstop=2:shiftwidth=2:expandtab:
