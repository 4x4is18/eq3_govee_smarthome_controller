#include "TheNetwork.h"

// Constructor
// Init the mqttClient with the wifiClient
TheNetwork::TheNetwork():mqttClient(wifiClient){

}

void TheNetwork::connectMQTT(const char* mqttServer, const int mqttPort) {
    mqttClient.setServer(mqttServer, mqttPort);
    reconnectMQTT();

}

// reconnect to MQTT Broker
void TheNetwork::reconnectMQTT() {
    while (!mqttClient.connected()) {
        if (mqttClient.connect("ESP32Client")) {
            Serial.println("MQTT connected");
        } else {
            delay(500);
            Serial.println("retry mqtt...");
        }
    }
}

void TheNetwork::connectBLE(){

  // Start the BLE Connection
  NimBLEDevice::init("");

  // no idea that it does. Maybe empower the BLE connection
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV,     ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN,    ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_CONN_HDL0, ESP_PWR_LVL_P9);

  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->setMinInterval(160); // 100 ms
  advertising->setMaxInterval(320); // 200 ms
  advertising->start();

    scan = NimBLEDevice::getScan();  //create new scan
    //scan->setAdvertisedDeviceCallbacks(&advCallbacks, true);
    //scan->setScanCallbacks(&scanCallbacks);
    scan->setActiveScan(true);     // active scan = request scan response
    scan->setInterval(48);
    scan->setWindow(48);
}

NimBLEScan* TheNetwork::getBLEScanner(){
  return scan;
}


void TheNetwork::loop() {
   if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    mqttClient.loop();
}

void TheNetwork::connectWiFi(const char* _ssid, const char* _password) {
    
  WiFi.begin(_ssid, _password);

   unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > 10000) {
            Serial.println("WiFi connection timeout!");
            return;
        }
        delay(300);
        Serial.println("Connecting WiFi...");
    }

    Serial.println("WiFi connected!");
    Serial.println(WiFi.localIP());

}


PubSubClient* TheNetwork::getMQTTClient() {
  return &mqttClient; 
}

// Sends messages to the mqtt Broker
bool TheNetwork::sendMqttMessage(const char* topic, const char* payload) {

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }

  if (topic == nullptr || payload == nullptr) {
    Serial.println("NULL topic or payload");
    return false;
  }

  return mqttClient.publish(topic, payload);

};



