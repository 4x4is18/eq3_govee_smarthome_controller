#include "Config.h"
#include "DeviceConfig.h"
#include "Thermostat.h"
#include "Thermostat.h"
#include "Thermometer.h"
#include "TheNetwork.h"
#include "Globals.h"
#include "Message.h"

// heater in the working room
Thermostat* buro;
Thermostat* wohn;

// Wifi, MQTT and BLE Management
TheNetwork networkBridge;

// Govee thermometer Management
//Thermometer thermometer();
Thermometer* thermometer;
Thermostat* t[2];

DeviceConfig deviceConfig;

unsigned long lastScanTime = 0; // helper for BLE scan

void setup() {
  Serial.begin(115200);

  // Init the two queues
  bleToMqttQueue = xQueueCreate(10, sizeof(Message));
  mqttToBleQueue = xQueueCreate(10, sizeof(Message));

  if (!bleToMqttQueue || !mqttToBleQueue) {
    Serial.println("Queue creation failed");
    while (true);
  }

  // Start Wifi, MQTT and BLE
  networkBridge.connectWiFi(WIFI_SSID,WIFI_PASSWORD);
  networkBridge.connectMQTT(MQTT_SERVER,MQTT_PORT);

  networkBridge.getMQTTClient()->setCallback(mqttCallback);
  networkBridge.connectBLE();

  // Start the Govee thermometer scanner
  thermometer = new Thermometer(networkBridge, thermoHygrometer, thermoHygrometerCount);
  thermometer->setBLEScanner(networkBridge.getBLEScanner());
  thermometer->scan(scanInterval);
  //   //init the heater
  // for (uint8_t i = 0; i < heizungsCount; i++) {
  //   t[i] = new Thermostat(heizungsThermostat[i], networkBridge.getMQTTClient());
  //   Serial.println(heizungsThermostat[i].name);
  //}
};

void loop() {

  // scan for new broadcast information
   unsigned long now = millis();
  if (now - lastScanTime >= scanPauseInterval) {
    lastScanTime = now;
    thermometer->scan(scanInterval);
  }

  networkBridge.getMQTTClient()->loop();

  Message msg;
  while (xQueueReceive(bleToMqttQueue, &msg, 0) == pdTRUE) {
    if (networkBridge.getMQTTClient()->connected()) {
      networkBridge.getMQTTClient()->publish(msg.topic, msg.payload);
    }
  }

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




