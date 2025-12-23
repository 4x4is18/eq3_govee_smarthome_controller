#ifndef THENETWORK_H
#define THENETWORK_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>

class TheNetwork {
  public:
    TheNetwork();
    void loop();
    void connectWiFi(const char* _ssid, const char* _password);
    void connectMQTT(const char* mqttServer, const int mqttPort);
    void connectBLE();
    PubSubClient* getMQTTClient();
    BLEScan* getBLEScanner();
    bool sendMqttMessage(const char* topic, const char* payload);

  private:
    void reconnectMQTT();
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    BLEScan* BLEScanner;
};

#endif