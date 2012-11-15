#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

// Function prototypes
void warmUp();
void initGsm();
bool gsmOk();
void haltWithGsmError();
void gsmReadLine(char *line);
void gsmSkipLine();
void loadThresholds();
int readThreshold(byte i);
void writeThreshold(byte i, int value);
bool displayTimedOut();
void handleButtonPress();
bool buttonPressed();
void deactivateDisplay();
void activateDisplay();
bool tempCheckNeeded();
void readNextTemp();
byte getRawData(byte addr[], byte data[]);
int dataToCelcius(byte data[], byte sensor_type);
bool getThreshold();
void checkTemps();
bool alarmSMSNeeded();
void sendAlarmSMS();
void updateDisplay();
void printTemp(int temp);
void smsTemp(int temp);
byte getSensorType(byte addr[]);

#endif

// vim:softtabstop=2:shiftwidth=2:expandtab:
