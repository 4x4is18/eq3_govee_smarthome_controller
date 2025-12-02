#ifndef THERMOMETER_H
#define THERMOMETER_H

// TODO: Do we need all of them?
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>
#include <BLEUtils.h>
#include <PubSubClient.h>

class Thermometer {
  public:
    Thermometer(PubSubClient* client);
    void setBLEScanner(BLEScan* _pBLEScan);
    void scan();

  private:

    // callback class. TODO: works for the moment. Ugly af
    class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    public:
        MyAdvertisedDeviceCallbacks(Thermometer* p) : parent(p) {}
        void onResult(BLEAdvertisedDevice advertisedDevice) override;

    private:
        Thermometer* parent;
    };

    void reconnectMQTT();
    BLEScan* pBLEScan;
    PubSubClient* mqttClient;
    void decodeGovee(int len, uint8_t* dp);
    void publishGovee(const char* message);
    const char* goveeToJson(float temperature, float humidity, int battery, const char* mac);
};

#endif
