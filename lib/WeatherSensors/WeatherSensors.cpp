
#include "WeatherSensors.h"
//#include <Arduino.h>

WeatherSensors::WeatherSensors(uint8_t dataPin, bool pairingRequired, bool ignoreRepeatedMessages, bool keepNewDataState)
{
    /*
    for (byte i = 0; i < 4; i++)
        if (__instance[i] == 0)
        {
            __instance[i] = this;
            DEBUG_PRINTLN("::");
            break;
        }    
    */
    this->dataPin = dataPin;
    this->pairingRequired = pairingRequired;
    this->ignoreRepeatedMessages = ignoreRepeatedMessages; 
    this->keepNewDataState = keepNewDataState;
}

WeatherSensors::~WeatherSensors()
{
    /*
    for (byte i = 0; i < 4; i++)
        if (__instance[i] == this)
        {
            __instance[i] = 0;
        }    
    */
}

void WeatherSensors::begin()
{
    // set up the pins!
    DEBUG_PRINTF("Data pin %d\r\n", RX433DATAPIN);
    pinMode(RX433DATAPIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(RX433DATAPIN), rx433Handler, CHANGE);

    DEBUG_PRINTLN("WeatherSensors begin");
}

void WeatherSensors::end()
{
    DEBUG_PRINTLN("WeatherSensors end");
    detachInterrupt(digitalPinToInterrupt(RX433DATAPIN));    
}

void WeatherSensors::pair(void (*pairedDeviceAdded)(struct SensorData *newData))
{
    Serial.println("1. WeatherSensors::pair");
    this->pair(NULL, 0, *pairedDeviceAdded);
}

void WeatherSensors::pair(struct SensorData pairedDevices[], byte pairedDevicesCount, void (*pairedDeviceAdded)(struct SensorData *newData))
{
    Serial.println("2. WeatherSensors::pair");
    
    this->pairedDeviceAdded = pairedDeviceAdded;

    if (pairedDevices == NULL)
    {
        DEBUG_PRINTLN("Start pairing for 1 min.");
        //pairingEndMillis = millis() + 60 * 1000L;
    }
    else
    {
        this->pairedDevices = (struct SensorData *)realloc(pairedDevices, pairedDevicesCount * sizeof(struct SensorData));
        memcpy(this->pairedDevices, pairedDevices, pairedDevicesCount);
    }
}


void WeatherSensors::pairingDevice(struct SensorData *data)
{
    Serial.println("WeatherSensors::pairingDevice");
    //allready paired
    if (isPaired(data))
        return;

    pairedDevices = (SensorData *)realloc(pairedDevices, ++pairedDevicesCount * sizeof(SensorData));
    pairedDevices[pairedDevicesCount - 1] = *data;
    DEBUG_PRINTF("New %s device with ID %d paired\r\n", (xBits < 3 ? "temperature" : (subID == 1 ? "wind speed" : (subID == 7 ? "wind direction and gust" : (subID == 3 ? "rain sensor" : "unknown")))), randomID);

    if (pairedDeviceAdded != NULL)
        pairedDeviceAdded(data);
}

void WeatherSensors::showPairedDevices()
{
    unsigned long currentMillies = millis();
    if (currentMillies - lastSensorPrintOutTime > SENSORSUMMARYTIME)
    {
        Serial.println();
        Serial.println("All currently paired devices");
        lastSensorPrintOutTime = currentMillies;
        for (int i = 0; i < pairedDevicesCount; i++)
        {
            Serial.printf("Sensor %d: %s\n", i, pairedDevices[i].sensorname);
        }
        Serial.println();
    }
}


bool WeatherSensors::isPaired(struct SensorData *data)
{
    for (int i = 0; i < pairedDevicesCount; i++)
    {
        if (pairedDevices[i].randomID == data->randomID)
        {
            return true;
        }
    }
    return false;
}

bool WeatherSensors::hasNewData(struct SensorData *data)
{
    unsigned long currentMillies = millis();
    if (!isPaired(data))
        return false;
    
    unsigned long diff = currentMillies - pairedDevices[i].lastMsgTime;
    bool somethingChanged = pairedDevices[i].batteryLow != data->batteryLow 
        || pairedDevices[i].humidity != data->humidity 
        || pairedDevices[i].temperature != data->temperature;

    if (diff > IGNOREDUPLICATES_RANGE || somethingChanged)
    {
        //Serial.printf("\nUpdate paired Device: %s\n", data->model);
        pairedDevices[i].batteryLow = data->batteryLow;
        pairedDevices[i].humidity = data -> humidity;
        pairedDevices[i].temperature = data -> temperature;
        pairedDevices[i].lastMsgTime = currentMillies;
        return true;
    }
    
    return false;
}

/*
// Helper for ISR call
WeatherSensors *WeatherSensors::__instance[4] = {0};

#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR void WeatherSensors::_ISR()
#else
void WeatherSensors::_ISR()
#endif
{
  
    for (byte i = 0; i < 4; i++)
        if (__instance[i])
        {
            __instance[i]->rx433Handler();
        }
}*/

volatile unsigned int pulsbuf[PBSIZE]; // ring buffer storing LOW pulse lengths
volatile unsigned int hibuf[PBSIZE]; // ring buffer storing HIGH pulse lengths
unsigned int validpulsbuf[MAXPULSECOUNT]; // linear buffer storing valid LOW pulses
unsigned int validhibuf[MAXPULSECOUNT];  // linear buffer storing valid HIGH pulses

volatile byte pbread,pbwrite;  // read and write index into ring buffer

#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR void WeatherSensors::rx433Handler()
#else
void WeatherSensors::rx433Handler()
#endif
{ // Interrupt-Routine
    static long rx433LineUp, rx433LineDown;
    long LowVal, HighVal;
    int rx433State = digitalRead(RX433DATAPIN); // current pin state
    if (rx433State) // pin is now HIGH
    {
        rx433LineUp=micros(); // line went HIGH after being LOW at this time
        LowVal=rx433LineUp - rx433LineDown; // calculate the LOW pulse time
        if (LowVal>MINBITPULSE)
        {   // store pulse in ring buffer only if duration is longer than MINBITPULSE
            // To be able to store startpulses of more than Maxint duration, we dont't store the actual time,
            // but we store  MINSTARTPULSE+LowVal/10, be sure to calculate back showing the startpulse length!
            if (LowVal>MINSTARTPULSE) 
                LowVal=MINSTARTPULSE+LowVal/10; // we will store this as unsigned int, so do range checking

            pulsbuf[pbwrite]=LowVal; // store the LOW pulse length
            pbwrite++;  // advance write pointer in ringbuffer
            if (pbwrite>=PBSIZE) 
                pbwrite=0; // ring buffer is at its end
        } 
    }
    else
    {
        rx433LineDown=micros(); // line went LOW after being HIGH
        HighVal=rx433LineDown - rx433LineUp; // calculate the HIGH pulse time
        if (HighVal>31999) 
            HighVal=31999; // we will store this as unsigned int
        hibuf[pbwrite]=HighVal; // store the HIGH pulse length
    }
}


//Analyze the Buffer
void WeatherSensors::analyzeBuffer()
{
    long lowtime, hitime;
    if (pbread!=pbwrite) // check for data in ring buffer
    {
        lowtime=pulsbuf[pbread]; // read data from ring buffer
        hitime=hibuf[pbread];
        cli(); // Interrupts off while changing the read pointer for the ringbuffer
        pbread++;
        if (pbread>=PBSIZE)
            pbread=0;
        sei(); // Interrupts on again
        if (lowtime>MINSTARTPULSE) // we found a valid startbit!
        {
            if (counting)
            {
                PrepareDataForStack(); // new buffer starts while old is still counting, show it first     
            }
            startBitDurationL=lowtime;
            startBitDurationH=hitime;
            counting=true;     // then start collecting bits
            counter=0;         // no data bits yet
        }
        else if (counting && (counter==0)) // we now see the first data bit
        { // this may be a 0-bit or a 1-bit, so make some assumption about max/min lengths of data bits that will follow
            shortBitDuration=lowtime/2;
            if (shortBitDuration<MINBITPULSE+PULSEVARIANCE)
                shortBitDuration=MINBITPULSE;
            else 
                shortBitDuration-=PULSEVARIANCE;
            longBitDuration=lowtime*2+PULSEVARIANCE;
            validpulsbuf[counter]=lowtime;
            validhibuf[counter]=hitime;
            counter++;
        }
        else if (counting&&(lowtime>shortBitDuration)&&(lowtime<longBitDuration))
        {
            validpulsbuf[counter]=lowtime;
            validhibuf[counter]=hitime;
            counter++;
            if ((counter==MAXPULSECOUNT) || (hitime<MINHITIME))
            {
                PrepareDataForStack();
            }
        }
        else // Low Pulse is too short
        {
            if (counting) 
            {
                PrepareDataForStack();
            }
            counting=false;
            counter=0;
        } 
    }    
}

void WeatherSensors::PrepareDataForStack()
{
    long sum;
    int avg;
    sum=0;
    if (counter>=MINPULSECOUNT)
    { // only show buffer contents if it has enough bits in it
        RXRawData rxRawData;
        rxRawData.startBitDurationL = (startBitDurationL-MINSTARTPULSE)*10L;
        rxRawData.startBitDurationH = startBitDurationH;
        rxRawData.counter = counter;
        rxRawData.rawData = 0;

        //Serial.println();
        //Serial.print("Start Bit L: "); Serial.print((startBitDurationL-MINSTARTPULSE)*10L);
        //Serial.print("   H: ");Serial.println(startBitDurationH);
        //Serial.print("Data Bits: ");Serial.println(counter);
        //Serial.print("L: ");
        for (i=0;i<counter;i++)
        {
            //Serial.print(validpulsbuf[i]);Serial.print(" ");
            rxRawData.validpulsbuf[i] = validpulsbuf[i];
            sum+=validpulsbuf[i];
        }
        //Serial.println();

        //Serial.print("H: ");
        for (i=0;i<counter;i++)
        {
            //Serial.print(validhibuf[i]);Serial.print(" ");
            rxRawData.validhibuf[i] = validhibuf[i];
        }
        //Serial.println();

        avg=sum/counter; // calculate the average pulse length
        //Serial.print("Average:");
        //Serial.println(avg);
        rxRawData.avg = sum/counter;
        // then assume that 0-bits are shorter than avg, 1-bits are longer than avg
        for (i=0;i<counter;i++)
        {
            bool currentbit = 0;
            char currentchar;
            if (validpulsbuf[i]<avg)
            {
                currentchar = '0';
                currentbit = 0;
            }
            else
            {
                currentchar = '1';
                currentbit = 1;
            }
            rxRawData.binData[i] = currentchar;

            //shift last position left
            rxRawData.rawData = rxRawData.rawData << 1;
            //add new Databit on the right to get the right direction
            rxRawData.rawData |= (unsigned long long) currentbit;
        }
        rxRawData.binData[i+1] = '\0';
        dataBuffer.push(&rxRawData);
        
        //Serial.println();
    }
    counting=false;
    counter=0;
}

