#ifndef WeatherSensors_h
#define WeatherSensors_h
#include <Arduino.h>
#include ".\helper\Ringbuffer.h"
#include ".\helper\datastructures.h"
#include ".\SignalDecoder\Decoder.h"


// Setup debug printing macros.
#ifdef WSDR_DEBUG
#define DEBUG_PRINT(...)                  \
    {                                     \
        DEBUG_PRINTER.print(__VA_ARGS__); \
    }
#define DEBUG_PRINTLN(...)                  \
    {                                       \
        DEBUG_PRINTER.println(__VA_ARGS__); \
    }
#define DEBUG_PRINTF(...)                  \
    {                                      \
        DEBUG_PRINTER.printf(__VA_ARGS__); \
    }
#else
#define DEBUG_PRINT(...) \
    {                    \
    }
#define DEBUG_PRINTLN(...) \
    {                      \
    }
#define DEBUG_PRINTF(...) \
    {                     \
    }
#endif

// connect data pin of rx433 module to a pin that can handle hardware interrupts
// with an Arduino UNO this is digital I/O pin 2 or 3 only
#ifndef RX433DATAPIN
    #define RX433DATAPIN 5
#endif
// Now make some suggestions about pulse lengths that may be detected
// minimum duration (microseconds) of the start pulse
#define MINSTARTPULSE 3800

// minimum duration (microseconds) of a short bit pulse
#define MINBITPULSE 450

// minimum duration (microseconds) of a HIGH pulse between valid bits
#define MINHITIME 50

// variance between pulses that should have the same duration
#define PULSEVARIANCE 250

// minimum count of data bit pulses following the start pulse
#define MINPULSECOUNT 20

// maximum count of data bit pulses following the start pulse
#define MAXPULSECOUNT 50

// buffer sizes for buffering pulses in the interrupt handler
#define PBSIZE 216

// Time to ignore multiple messages from one Device
#define IGNOREDUPLICATES_RANGE 1000

// Time to write out all paired Sensors
#define SENSORSUMMARYTIME 120000

class WeatherSensors
{
    public:
        Ringbuffer<RXRawData, 20> dataBuffer;     // Ringbuffer für zuletzt empfangene Daten (10 x 32Bit)
        unsigned long lastSensorPrintOutTime = 0;
        
        WeatherSensors(uint8_t dataPin, bool pairingRequired = false, bool ignoreRepeatedMessages = false, bool keepNewDataState = false);
        ~WeatherSensors();
        void begin();
        void end();   
                
        void pair(void (*pairedDeviceAdded)(struct SensorData *newData) = NULL);
        void pair(struct SensorData pairedDevices[], byte pairedDevicesCount, void (*pairedDeviceAdded)(struct SensorData *newData) = NULL);
        bool isPaired(struct SensorData *data);
        bool hasNewData(struct SensorData *data);
        void pairingDevice(struct SensorData *data);

        void showPairedDevices();
        
        void analyzeBuffer();
        void PrepareDataForStack();

    private:
        //static WeatherSensors *__instance[4];
        boolean counting;
        byte i,counter;
        int startBitDurationL,startBitDurationH,shortBitDuration,longBitDuration;    

        uint8_t dataPin;
        bool pairingRequired;
        
        byte pairedDevicesCount = 0;
        struct SensorData *pairedDevices = NULL;
        void (*pairedDeviceAdded)(struct SensorData *newData) = NULL;  
        bool ignoreRepeatedMessages;
        bool keepNewDataState;

    protected:   
        //static void _ISR();
        static void rx433Handler();
};

#endif