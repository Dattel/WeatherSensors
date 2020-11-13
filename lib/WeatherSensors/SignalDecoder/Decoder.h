#ifndef Decoder_h
#define Decoder_h

//#include <Arduino.h>
#include "DecoderStub.h"
#include "Decoder_SD_WS07.h"
#include "Decoder_TCM97001.h"

class Decoder{
    public:
             
        Decoder()
        {

        }
        void Init()
        {
            Add(new Decoder_SD_WD07);
            Add(new Decoder_TCM97001);
        }

        void PrintRegisteredDecoder()
        {
            Serial.println("Active Sensor Decoders:");
            for (int i = 0;i<registeredDecoderCount;i++)
            {
                DecoderStub *dec = (DecoderStub*)registeredDecoder[i];
                Serial.printf("Sensorname %d: %s\n", i, "DecodeName");
            }
              //Serial.printf("Sensor %d: %d\n", i, registeredDecoder[i].DecoderName);
        }

        SensorData *ParseSignal(RXRawData *data)
        {
            for (int i = 0;i< registeredDecoderCount;i++)
            {
                if (registeredDecoder[i]->Is(data))
                    return registeredDecoder[i]->Decode(data);
            }
            return NULL;
        }

    private:
        byte registeredDecoderCount = 0;
        //DecoderStub **registeredDecoder = NULL;    
        DecoderStub** registeredDecoder = new DecoderStub*[5];

        void Add(DecoderStub *newDecoder)
        {
            //registeredDecoder = (DecoderStub **)realloc(registeredDecoder, ++registeredDecoderCount * sizeof(DecoderStub));
            //registeredDecoder[registeredDecoderCount - 1] = newDecoder;
            registeredDecoder[registeredDecoderCount++] = newDecoder;
            
        }

};
#endif