#include "Thermometer.h"

#include <vector>    // <--- WICHTIG für std::vector

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) 
  {

    if (advertisedDevice.getAddress().toString().equals("a4:c1:38:5e:c4:6e")) 
    {
      Serial.println(advertisedDevice.getAddress().toString());

    } else {
      return;
    }
    int len=advertisedDevice.getPayloadLength();
    uint8_t* dp=advertisedDevice.getPayload();

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
      Serial.printf("T %3.1f H %3.1f B %d\n",Temperature,Humidity,Battery);

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
      Serial.printf("T %3.1f H %3.1f B %d\n",Temperature,Humidity,Battery);
      }
      i+=flen-1;
   }
  }
};

Thermometer::Thermometer(){
    
}

void Thermometer::setBLEScanner(BLEScan* _pBLEScan){
  pBLEScan = _pBLEScan;
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(),true);
}

void Thermometer::scan(){
  pBLEScan->start(5, false);
  //Serial.print("Devices found: ");
  //Serial.println(foundDevices->getCount());
  //Serial.println("Scan done!");
  pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory
}