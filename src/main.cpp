#include <Arduino.h>
#include "WeatherSensors.h"
#include ".\helper\helper.h"
//#include "SignalDecoder/Decoder.h"


#ifdef ESP8266
  #define RX433DATAPIN D5
  #define LED_PIN D2
#else
  #define RX433DATAPIN 5
  #define LED_PIN 2
#endif

#define WSDR_DEBUG = 1
WeatherSensors wsdr(RX433DATAPIN, false, false, false);

void PairedDeviceAdded(struct SensorData *newData)
{
  Serial.println("PairedDeviceAdded");
#if defined(ESP8266) || defined(ESP32)
    Serial.printf("New device paired %s\r\n", newData->sensorname);
#else
    Serial.print("New device paired ");
    Serial.println(newID, DEC);
#endif
    wsdr.pair(PairedDeviceAdded);

    // If you already have a sensor ID for your device, you can set it as follows.
    // byte myDeviceIDs[] = {34, 63};
    // wsdr.pair(myDeviceIDs, sizeof(myDeviceIDs));
}

Decoder d;

void setup() {
    if (LED_PIN != 0)
    {
      pinMode(LED_PIN,OUTPUT);
    }
    Serial.begin(115200);
    delay(2000);
    Serial.println("WeatherSensors Test");
    wsdr.begin();
    wsdr.pair(PairedDeviceAdded);
   
    d.Init();    

    //d.PrintRegisteredDecoder();
}

void printSensorData(SensorData *data)
{
  char * value = long_to_binary(data->randomID);
  Serial.println();
  Serial.printf("SensorID: %u Binary: %s\n", data->randomID, value);
  free(value);
  Serial.printf("  Model: %s\n",data->sensorname);

  value = long_to_binary(data->batteryLow);
  Serial.printf("  BatteryLow: %d Binary: %s\n", data->batteryLow,value);
  free(value);
  value = long_to_binary(data->channel);
  Serial.printf("  Channel: %d Binary: %s\n", data->channel,value);
  free(value);
  value = long_to_binary(data->temperature);
  Serial.printf("  Temp: %.2f Binary: %s\n", data->temperature,value);
  free(value);
  value = long_to_binary(data->humidity);
  Serial.printf("  Hum: %.2f Binary: %s\n", data->humidity,value);
  free(value);
  Serial.println();
}

void loop() {
    //unsigned long long data =  wsdr.readData();
    wsdr.analyzeBuffer();

    if (wsdr.dataBuffer.currentSize() > 0)
    {
      RXRawData rxData;
      noInterrupts();
      wsdr.dataBuffer.pull(rxData);
      interrupts();
      wsdr.showPairedDevices();
      
      SensorData *data = d.ParseSignal(&rxData);
      if (data == NULL)
      {
        Serial.printf("Unknown Sensor data -> %llu : %s \n", rxData.rawData, long_to_binary(rxData.rawData));
        return;
      }
      
      digitalWrite(LED_PIN,HIGH);
      if (!wsdr.isPaired(data))
      {
        Serial.printf("\nPairing new sensor: %s\n", data->sensorname);
        wsdr.pairingDevice(data);
      }
      
      if (wsdr.hasNewData(data))
      {
        printSensorData(data);
      }
      else
        Serial.print(".");
      
      free(data);
      
      digitalWrite(LED_PIN,LOW);
    }
}

