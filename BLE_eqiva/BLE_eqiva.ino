#include "Config.h"
#include "DeviceConfig.h"
#include "Thermostat.h"
#include "Thermostat.h"
#include "Thermometer.h"
#include "TheNetwork.h"

// heater in the working room
Thermostat* buro;
Thermostat* wohn;

// Wifi, MQTT and BLE Management
TheNetwork networkBridge;
BLEScan* pBLEScan;

// Govee thermometer Management
//Thermometer thermometer();
Thermometer* thermometer;
Thermostat* t[5];


DeviceConfig deviceConfig;

void setup() {
  Serial.begin(115200);

  // Start Wifi, MQTT and BLE
  networkBridge.connectWiFi(WIFI_SSID,WIFI_PASSWORD);
  networkBridge.connectMQTT(MQTT_SERVER,MQTT_PORT);

  networkBridge.getMQTTClient()->setCallback(mqttCallback);
  networkBridge.connectBLE();

  // Start the Govee thermometer scanner
  thermometer = new Thermometer(networkBridge.getMQTTClient(), thermoHygrometer, deviceCount);
  thermometer->setBLEScanner(networkBridge.getBLEScanner());

  //init the heater
for (uint8_t i = 0; i < heizungsCount; i++) {
  t[i] = new Thermostat(heizungsThermostat[i], networkBridge.getMQTTClient());
  Serial.println(heizungsThermostat[i].name);
}
  
};



void loop() {

  // scan for new broadcast information
  //thermometerWohn->scan();
  thermometer->scan();
  delay(5000);

  networkBridge.getMQTTClient()->loop();

}

// MQTT Callback
void mqttCallback(char* topic, byte* payload, unsigned int length) {
     String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    for (int i = 0; i < heizungsCount; i++) {

        // find the id (last two mac adress blocks) in the topic
        if (String(topic).indexOf(heizungsThermostat[i].id) >= 0) {
            Serial.print("Thermostat gefunden: ");
            Serial.println(heizungsThermostat[i].name);
            t[i]->handleCommand(topic, payload, length);

        }
    }

  Serial.println(message);
}




