#ifndef datastructures_h
#define datastructures_h

//#include <Arduino.h>

struct SensorData
{
    char sensorname[32];
    unsigned char randomID;
    bool batteryLow;
    char channel;
    float temperature;
    float humidity;
    unsigned long lastMsgTime;
};

struct RXRawData
{
    uint64_t rawData;
    uint16_t startBitDurationL;
    uint16_t startBitDurationH;
    uint8_t counter;
    uint16_t validpulsbuf[50];
    uint16_t validhibuf[50];
    char binData[50];
    uint16_t avg;
};

#endif