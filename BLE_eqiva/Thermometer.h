#ifndef THERMOMETER_H
#define THERMOMETER_H

// TODO: Do we need all of them?
#include <NimBLEDevice.h> // main NimBLE library
#include <map>
#include <tuple>
#include "TheNetwork.h"
#include "DeviceConfig.h"
#include "Globals.h"

class Thermometer {
  public:
    Thermometer(TheNetwork& networkBridge, DeviceConfig* deviceConfig, int deviceCount);
    void setBLEScanner(NimBLEScan* _pBLEScan);
    void scan(const unsigned long scanInterval);

  private:

    // Scan callback
    class MyScanCallbacks : public NimBLEScanCallbacks {
    public:
        MyScanCallbacks(Thermometer* parent) : parent(parent) {}
        void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override;
        void onScanEnd(const NimBLEScanResults& results, int reason) override;

    private:
        Thermometer* parent;
    };

    MyScanCallbacks scanCallbacks{this};


    DeviceConfig* deviceConfig;
    int deviceCount;
    void reconnectMQTT();
    NimBLEScan* pBLEScan;
    TheNetwork& networkBridge;
    bool decodeGovee(int len, const uint8_t* dp, int deviceIndex,float& outTemperature, float& outHumidity, int& outBattery);
    void publishGovee(float temperature, float humidity, int battery, int deviceIndex);
    std::map<int, std::tuple<float, float, int>> scanResults;  // Temporary storage for duplicate filtering
};

#endif
