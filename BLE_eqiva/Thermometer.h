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
#include "TheNetwork.h"
#include "DeviceConfig.h"

class Thermometer {
  public:
    Thermometer(TheNetwork& networkBridge, DeviceConfig* deviceConfig, int deviceCount);
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
    DeviceConfig* deviceConfig;
    int deviceCount;
    void reconnectMQTT();
    BLEScan* pBLEScan;
    TheNetwork& networkBridge;
    void decodeGovee(int len, uint8_t* dp, int deviceIndex);
    void publishGovee(float temperature, float humidity, int battery, int deviceIndex);
};

#endif
