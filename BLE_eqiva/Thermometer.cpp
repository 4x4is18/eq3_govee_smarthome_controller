#include "Thermometer.h"

Thermometer::Thermometer(TheNetwork& _networkBridge, DeviceConfig* _deviceConfig, int _deviceCount): networkBridge(_networkBridge)
{
    deviceConfig = _deviceConfig;
    deviceCount = _deviceCount;
}

void Thermometer::setBLEScanner(BLEScan* _pBLEScan){
  pBLEScan = _pBLEScan;
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(this),true);
}

void Thermometer::scan(){
  pBLEScan->start(5, false);
  pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory
}


// publish govee values to the networkBridge
void Thermometer::publishGovee(float temperature, float humidity, int battery, int deviceIndex)
{
    // ---- Check 1: Check if deviceConfig and deviceIndex are correct ----
    if (!deviceConfig) return;
    if (deviceIndex < 0) return;

    // check if you can build the topic string with room and type
    const char* room = deviceConfig[deviceIndex].room;
    const char* type = deviceConfig[deviceIndex].deviceType;
    if (!room || !type) return;

    // helper variables
    char topic[128];
    char payload[32];

    // ---- TEMPERATURE ----
    snprintf(topic, sizeof(topic),"home/%s/%s/temperature", room, type);
    snprintf(payload, sizeof(payload),"%.1f", temperature);
    networkBridge.sendMqttMessage(topic, payload);

    // ---- HUMIDITY ----
    snprintf(topic, sizeof(topic),"home/%s/%s/humidity", room, type);
    snprintf(payload, sizeof(payload),"%.1f", humidity);
    networkBridge.sendMqttMessage(topic, payload);

    // ---- BATTERY ----
    snprintf(topic, sizeof(topic),"home/%s/%s/battery", room, type);
    snprintf(payload, sizeof(payload),"%d", battery);
    networkBridge.sendMqttMessage(topic, payload);
}

void Thermometer::decodeGovee(int len,uint8_t* dp, int deviceIndex){

      for (int i = 0; i < len; i++) 
    {
     // Serial.printf("i %d len %d\n",i,len);
      int flen,ftype; 
      flen=dp[i++]-1; //0
      ftype=dp[i++]; //1
      // fdata at dp[i], size flen
      for(int j=0;j<flen;j++)
      {
        if((j+i)>=len) break; 
        int c=dp[j+i];
       // if(c>=32 && c<127) Serial.printf("%c",c );
       // else               Serial.printf("[%X]",c );
      }
      if(flen==8 && ftype==0xFF && dp[i+0]==0x88 && dp[i+1]==0xec)
      {
      int iTemp = int(dp[i+3]) << 16 | int(dp[i+4]) << 8 | int(dp[i+5]);
      bool bNegative = iTemp & 0x800000;	// check sign bit
      iTemp = iTemp & 0x7ffff;			// mask off sign bit
      float Temperature = float(iTemp / 1000) / 10.0; // issue #49 fix. 
        // After converting the hexadecimal number into decimal the first three digits are the 
        // temperature and the last three digits are the humidity.So "03519e" converts to "217502" 
        // which means 21.7 °C and 50.2 % humidity without any rounding.
      if (bNegative)						// apply sign bit
          Temperature = -1.0 * Temperature;
      float	Humidity = float(iTemp % 1000) / 10.0;
      int Battery = int(dp[i+6]);
      publishGovee(Temperature,Humidity,Battery,deviceIndex);

      }
      else
      if(flen==9 && ftype==0xFF && dp[i+0]==0x88 && dp[i+1]==0xec)
      {
        // This data came from https://github.com/neilsheps/GoveeTemperatureAndHumidity
        // 88EC00 0902 CD15 64 02 (Temp) 41.378°F (Humidity) 55.81% (Battery) 100%
        // 2 3 4  5 6  7 8  9
        short iTemp = short(dp[i+4]) << 8 | short(dp[i+3]);
        int iHumidity = int(dp[i+6]) << 8 | int(dp[i+5]);
        float Temperature = float(iTemp) / 100.0;
        float Humidity = float(iHumidity) / 100.0;
        int Battery = int(dp[i+7]);
      publishGovee(Temperature,Humidity,Battery,deviceIndex);
      }
      i+=flen-1;
   }
}

void Thermometer::MyAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
    
    bool unknownDevice = true;
    int deviceIndex = -1;
      // Schleife über alle Devices
    for (int i = 0; i < parent->deviceCount; i++) {
          if (advertisedDevice.getAddress().toString().equals(parent->deviceConfig[i].macAddress)){
          deviceIndex = i;
          unknownDevice = false;
          }
    }
    
    if(unknownDevice || deviceIndex == -1) {
      return;
    }

    int len = advertisedDevice.getPayloadLength();
    uint8_t* dp = advertisedDevice.getPayload();
    parent->decodeGovee(len, dp, deviceIndex);
}


