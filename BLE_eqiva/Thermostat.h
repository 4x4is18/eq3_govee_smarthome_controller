#ifndef THERMOSTAT_H
#define THERMOSTAT_H


#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>
#include <PubSubClient.h>
#include "DeviceConfig.h"

class Thermostat {
  public:
    Thermostat(DeviceConfig& _deviceConfig, PubSubClient* client);
    void setTemperature(float temperature);
    void setLock(boolean lock);
    String getThermostatID();
    void handleCommand(char* topic, byte* payload, unsigned int length);
 
  private:
  bool connectToThermostat();
  bool setupCharacteristic();
  void publishStatus(const char* message);
  const char* toJson(bool isLocked, String mode, bool windowOpen, uint8_t valvePercent, float temperature);
  String extractCommand(const char* topic);
  String mac;
  DeviceConfig& deviceConfig;
  BLEUUID serviceUUID;
  BLEUUID writeUUID;
  BLEUUID notifyUUID;
  BLEClient* pClient;
  BLERemoteCharacteristic* writeChar;
  BLERemoteCharacteristic* notifyChar;
  PubSubClient* mqttClient;
};

#endif

