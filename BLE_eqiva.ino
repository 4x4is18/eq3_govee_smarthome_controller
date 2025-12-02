#include "Config.h"
#include "Thermostat.h"
#include "Thermostat.h"
#include "Thermometer.h"
#include "TheNetwork.h"


Thermostat* buro;
TheNetwork networkBridge;
BLEScan* pBLEScan;
Thermometer thermometer;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Client...");

  networkBridge.connectWiFi(WIFI_SSID,WIFI_PASSWORD);
  networkBridge.connectMQTT(MQTT_SERVER,MQTT_PORT);
  networkBridge.connectBLE();
  thermometer.setBLEScanner(networkBridge.getBLEScanner());
  buro = new Thermostat(EQIVA_BURO, networkBridge.getMQTTClient());

  //pBLEScan = networkBridge.getBLEScanner();
  //pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(),true);

  delay(5000); // 5 Sekunden warten
  buro->setTemperature(19);
  
}

void loop() {
  thermometer.scan();
  delay(5000); // 5 Sekunden warten


}






