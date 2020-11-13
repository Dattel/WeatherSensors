#ifndef Decoder_SD_WD07_h
#define Decoder_SD_WD07_h

//#include <Arduino.h>
//#include "Decoder.h"
//#include "helper/datastructures.h"

class Decoder_SD_WD07 : public DecoderStub
{
    // 36 Bits
    // AAAA BBBB C D EE FF GGGGGGGGGG HHHH IIIIIIII
    // A    4x ID0
    // B    4x ID1
    // C    1x BatteryLow  -> BatteryLow=0
    // D    1x Manual transmitted?
    // E    2x Channel  -> Channel 1 = 00, Channel 2 = 01, Channel 3 = 10
    // F    2x Sign for negative values
    // G    10x Temperature  -> temp is 12 bit signed scaled by 10 
    // H    4x const is always 1111 (0xF) or 1010 (0xA)
    // I    8x Humidity -> always 00000000

    //Start Bit L: 8960   H: 640
    //Data Bits: 36
    //L: 1762 812 827 824 819 844 845 821 1823 820 859 1823 854 880 873 872 1847 1860 1847 869 890 883 882 881 1875 1878 1862 1856 898 904 887 885 905 899 894 895 
    //H: 589 656 645 632 652 622 625 634 620 651 610 609 615 590 593 585 599 585 585 601 578 582 578 586 571 568 568 589 572 564 568 584 563 571 560 574 

    // Auriol HG00073
    //Start Bit L: 3890   H: 543
    //Data Bits: 36
    //L: 1928 918 1927 917 912 1923 1924 1933 913 923 914 921 916 926 922 928 1928 1927 1927 1934 1931 920 1929 1933 1923 1925 1928 1923 918 921 921 927 926 926 923 929 
    //H: 542 541 540 543 548 545 545 540 548 538 546 537 544 539 536 538 540 538 545 533 537 540 538 540 547 542 542 541 545 540 538 543 533 535 538 542


    public:
    Decoder_SD_WD07()
    {
        //strcpy(DecoderName, "Decoder_SD_WD07\n");
    }
    
    virtual bool Is(RXRawData *data) const
    {
        //Bit 10 ist immer 0 
        //Bit 25-28  sind immer 1
        byte O_Bit = data->rawData >> 26 & 0x1;
        uint16_t X_Bits = data->rawData >> 8  & 0xf;
        //Serial.printf("O_bit: %d\n", O_Bit);
        //Serial.printf("X_bit: %d\n", X_Bits);
        if (O_Bit == 0 && X_Bits == 15)
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
        sensor->randomID = data->rawData >> 28 & 0xff;     
        // 1bit Battery
        sensor->batteryLow = data->rawData >> 27 & 0x1;     
        //1 bit 0

        // 2bit Channel
        sensor->channel = (data->rawData >> 24 & 0x3) + 1;
        //Temperatur
        //12 bit Temp -> 1111 1111 1111 -> 0xFFF
        long temp = data->rawData >> 12 & 0x0fff;
        short temperatur = temp;
        //Negative bit?
        if (temperatur& 0x0800){
            temperatur = temperatur | 0xf000;
        }
        sensor->temperature = ((float)temperatur/10);

        sensor->humidity = (data->rawData & 0xff)/10;     // 8bit Humidiy
        
        strcpy(sensor->sensorname, "SD_WS07");
        if (sensor->humidity == 0)
            strcat(sensor->sensorname, "_T_");
        else
            strcat(sensor->sensorname, "_TH_");
        
        char buffer[2];
        itoa (sensor->channel,buffer,0xd);
        strcat(sensor->sensorname, buffer);
        sensor->lastMsgTime = millis();
        return sensor;
    }
};
#endif