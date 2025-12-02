#ifndef THERMOMETER_H
#define THERMOMETER_H

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>
#include <BLEUtils.h>



class Thermometer {
  public:
    Thermometer();
    void setBLEScanner(BLEScan* _pBLEScan);
    void scan();

  private:
    void reconnectMQTT();
    BLEScan* pBLEScan;

};

#endif
