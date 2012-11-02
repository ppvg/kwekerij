#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

// Function prototypes
bool tempCheckNeeded();
void getTemps();
byte getRawData(byte addr[], byte data[]);
int dataToCelcius(byte data[], byte sensor_type);
void printTemps();
bool getThreshold();
void updateDisplay(bool changed);
bool displayTimedOut();
byte getSensorType(byte addr[]);

#endif
