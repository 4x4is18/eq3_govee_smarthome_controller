#ifndef THERMOSTAT_H
#define THERMOSTAT_H


#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>
#include <PubSubClient.h>

class Thermostat {
  public:
    Thermostat(String _mac, PubSubClient* client);
    void setTemperature(float temperature);
    void setLock(boolean lock);
 
  private:
  bool connectToThermostat();
  bool setupCharacteristic();
  void publishStatus(const char* message);
  const char* toJson(bool isLocked, String mode, bool windowOpen, uint8_t valvePercent, float temperature);
  String mac;
  BLEUUID serviceUUID;
  BLEUUID writeUUID;
  BLEUUID notifyUUID;
  BLEClient* pClient;
  BLERemoteCharacteristic* writeChar;
  BLERemoteCharacteristic* notifyChar;
  PubSubClient* mqttClient;
  

};

#endif

