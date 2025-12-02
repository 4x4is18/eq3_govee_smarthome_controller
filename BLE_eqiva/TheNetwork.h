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

  private:
    void reconnectMQTT();
    WiFiClient espClient;
    PubSubClient client;
    BLEScan* BLEScanner;
};

#endif