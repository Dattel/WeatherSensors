#ifndef DecoderStub_h
#define DecoderStub_h

//#include <Arduino.h>
//#include "helper/datastructures.h"

class DecoderStub{
    public:
    //char DecoderName[20];
    virtual bool Is(RXRawData *data) const = 0;
    virtual SensorData *Decode(RXRawData *data) const = 0;
};
#endif