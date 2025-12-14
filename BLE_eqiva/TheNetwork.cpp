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
  BLEDevice::init("");
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

    client.publish("test/test", "Hallo MQTT!");
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