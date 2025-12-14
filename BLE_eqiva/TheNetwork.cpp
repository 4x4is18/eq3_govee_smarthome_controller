#include "TheNetwork.h"

TheNetwork::TheNetwork():client(espClient){}

void TheNetwork::connectMQTT(const char* mqttServer, const int mqttPort) {
    client.setServer(mqttServer, mqttPort);
    reconnectMQTT();

}

void TheNetwork::reconnectMQTT() {
    while (!client.connected()) {
        if (client.connect("ESP32Client")) {
            Serial.println("MQTT connected");
        } else {
            delay(500);
            Serial.println("retry mqtt...");
        }
    }
}

void TheNetwork::connectBLE(){
  BLEDevice::init("BLE");
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV,     ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN,    ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_CONN_HDL0, ESP_PWR_LVL_P9);

  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->setMinInterval(160); // 100 ms
  advertising->setMaxInterval(320); // 200 ms
  advertising->start();

  BLEScanner = BLEDevice::getScan();  //create new scan

  BLEScanner->setActiveScan(true);  //active scan uses more power, but get results faster
  BLEScanner->setInterval(100);
  BLEScanner->setWindow(99);  // less or equal setInterval value
}

BLEScan* TheNetwork::getBLEScanner(){
  return BLEScanner;
}


void TheNetwork::loop() {
   if (!client.connected()) {
        reconnectMQTT();
    }

    client.loop();
    
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

PubSubClient* TheNetwork::getMQTTClient() { return &client; }