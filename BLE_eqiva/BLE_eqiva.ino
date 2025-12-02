#include "Config.h"
#include "Thermostat.h"
#include "Thermostat.h"
#include "Thermometer.h"
#include "TheNetwork.h"

// heater in the working room
Thermostat* buro;

// Wifi, MQTT and BLE Management
TheNetwork networkBridge;
BLEScan* pBLEScan;

// Govee thermometer Management
Thermometer* thermometer;

void setup() {
  Serial.begin(115200);

  // Start Wifi, MQTT and BLE
  networkBridge.connectWiFi(WIFI_SSID,WIFI_PASSWORD);
  networkBridge.connectMQTT(MQTT_SERVER,MQTT_PORT);
  networkBridge.connectBLE();

  // Start the Govee thermometer scanner
  thermometer = new Thermometer(networkBridge.getMQTTClient());
  thermometer->setBLEScanner(networkBridge.getBLEScanner());

  //init the heater
  buro = new Thermostat(EQIVA_BURO, networkBridge.getMQTTClient());
  
}

void loop() {

  // scan for new broadcast information
  thermometer->scan();
  delay(5000);


}






