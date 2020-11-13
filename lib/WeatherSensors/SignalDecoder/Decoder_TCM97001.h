#ifndef Decoder_TCM97001_h
#define Decoder_TCM97001_h

//#include <Arduino.h>
//#include "Decoder.h"
//#include "helper/datastructures.h"

class Decoder_TCM97001 : public DecoderStub
{
    // 36 Bits
    // AAAA AAAA B CC D EEEE EEEE EEEE FFFF FFFF GGGG
    // A    8x Random Bits
    // B    1x BatteryLow
    // C    2x xBits -> For Wind & RainSensors everytime 11 -> xBits = 3  
        // xBits = 3 ist Wind- oder Regensensor 
        // die Bits 12, 13, 14 enthalten eine Sub-ID (Windmesser sendet 2 Telegramme Sub-ID 1 und 7)
        // subID = 1 ist der Windsensor (Durchschnitts-Windgeschwindigkeit)
        // subID = 7 ist der Windsensor (Windrichtung und Windboen)
        // subID = 3 ist der Regensensor (absolute Regenmenge seit Batteriewechsel)
    // D    1x Manual
    // E    12x Temperature in 0.1 steps
    // F    8x Humidiy -> 4x einer 4x zehner
    // G    Checksum?
    
    
    // 36 Bits
    // AAAA BBBBBBBB C D EE FF GGGGGGGGGG HHHHHHHH
    // A    4x StartBits -> Prologue starts everytime with 1001?
    // B    8x Random
    // C    1x BatteryLow  -> BatteryLow=1
    // D    1x Manual transmitted?
    // E    2x Channel  -> Channel 1 = 00, Channel 2 = 01, Channel 3 = 10
    // F    2x Sign for negative values
    // G    10x Temperature -> temp is 12 bit signed scaled by 10 
    // H    8x Humidity -> always 00000000

    //Start Bit L: 8580   H: 656
    //Data Bits: 36
    //L: 3755 1812 1812 3794 1834 1814 1814 1883 1828 1816 3837 1910 1847 1836 1835 3818 1869 1822 1827 1885 3772 3732 3756 1884 1872 3728 1824 3825 1869 1832 1909 1892 1861 1918 1828 1984
    //H: 558 570 568 567 568 570 566 567 573 566 547 540 556 540 548 543 532 563 550 567 542 557 539 566 532 565 554 560 536 548 563 559 540 552 551 559 


    public:
    Decoder_TCM97001()
    {
        //strcpy(DecoderName, "Decoder_TCM97001\n");
    }
    
    virtual bool Is(RXRawData *data) const
    {
        //Bit 32-36 ist immer 1001
        uint16_t ID = data->rawData >> 32  & 0xf;
        //Serial.printf("O_bit: %d\n", O_Bit);
        //Serial.printf("X_bit: %d\n", X_Bits);
        if (ID == 9)
            return true;
        return false;
    }

    virtual SensorData *Decode(RXRawData *data) const
    {
        if (data->counter != 36)
            return NULL;

        SensorData *sensor;
        sensor = (SensorData*)malloc(sizeof(SensorData));

        // 8bit RandomID
        sensor->randomID = (data->rawData >> 24 & 0xff);
        // 1bit Battery
        sensor->batteryLow =  data->rawData >> 23 & 0x1;     
        
        //1 bit Manual??

        // 2bit Channel
        sensor->channel = data->rawData >> 20 & 0x3;     
        sensor->channel = sensor->channel +1 ;
        
        //2 bit NEG?
        
        // 10bit Temperatur
        long temp = (data->rawData >> 8 & 0x3ff);
        sensor->temperature = (float)temp /10;     
        // 8bit Humidiy
        sensor->humidity = (data->rawData & 0xff ) /10;

        
        strcpy(sensor->sensorname, "Prologue");
        if (sensor->humidity == 0)
            strcat(sensor->sensorname, "_T_");
        else
            strcat(sensor->sensorname, "_TH_");
        
        char bufferChannel[2];
        itoa (sensor->channel,bufferChannel,0xd);
        strcat(sensor->sensorname, bufferChannel);

        /*
        unsigned long long id = (input >> 28 & 0xff);
        char bufferID[4];
        itoa (id,bufferID, 0xd);
        strcat(sensor->model, bufferID);
        */        
        
        sensor->lastMsgTime = millis();
        return sensor;   
    }
};
#endif