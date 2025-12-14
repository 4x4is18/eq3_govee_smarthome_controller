#include "Thermostat.h"

#define SERVICE_UUID "3e135142-654f-9090-134a-a6ff5bb77046"
#define WRITE_UUID "3fa4585a-ce4a-3bad-db4b-b8df8179ea09"
#define NOTIFY_UUID "d0e8434d-cd29-0996-af41-6c90f4e0eb2a"

Thermostat::Thermostat(DeviceConfig& _deviceConfig, PubSubClient* client) : deviceConfig(_deviceConfig) {
  BLEUUID _serviceUUID(SERVICE_UUID);
  serviceUUID = _serviceUUID;
  BLEUUID _writeUUID(WRITE_UUID);
  writeUUID = _writeUUID;
  BLEUUID _notifyUUID(NOTIFY_UUID);
  notifyUUID = _notifyUUID;
  pClient = nullptr;
  writeChar = nullptr;
  notifyChar = nullptr;
  mqttClient = client; 

  char topic[100];
  snprintf(topic, sizeof(topic),"/home/%s/%s/%s/#",deviceConfig.room,deviceConfig.deviceType,deviceConfig.id);
  mqttClient->subscribe(topic);
  
  if (!connectToThermostat()) return;
  if (!setupCharacteristic()) return;

}

bool Thermostat::connectToThermostat() {
    pClient = BLEDevice::createClient();
    if (!pClient->connect(BLEAddress(deviceConfig.macAddress))) {
        return false;
    }
    return true;
}

bool Thermostat::setupCharacteristic() {
    BLERemoteService* pService = pClient->getService(serviceUUID);
    if (!pService) {
        pClient->disconnect();
        return false;
    }
    writeChar = pService->getCharacteristic(writeUUID);
    if (!writeChar) {
        pClient->disconnect();
        return false;
    }
    notifyChar = pService->getCharacteristic(notifyUUID);
    if (!notifyChar) {
        pClient->disconnect();
        return false;
    }
    if (writeChar->canWrite()) {
        uint8_t initVal = 0x03;
        writeChar->writeValue(&initVal, 1, true);
    }

    // Notification registrieren
    if (notifyChar->canNotify()) {
        notifyChar->registerForNotify([this](BLERemoteCharacteristic* rc, uint8_t* data, size_t length, bool isNotify) {
            if (length < 6) return;

            bool isLocked = data[1] == 0x20;

            // Modus
            uint8_t modeByte = data[2];
            String mode;
            switch (modeByte) {
                case 0x01: mode = "Auto"; break;
                case 0x02: mode = "Manual"; break;
                case 0x03: mode = "Boost"; break;
                default:   mode = "Unknown"; break;
            }

            bool windowOpen = data[3] == 0x01;
            uint8_t valvePercent = data[4];
            float temperature = data[5] / 2.0;
            Serial.print(deviceConfig.name);
            Serial.print(" |Locked: "); Serial.print(isLocked ? "Yes" : "No");
            Serial.print(" | Mode: "); Serial.print(mode);
            Serial.print(" | Window Open: "); Serial.print(windowOpen ? "Yes" : "No");
            Serial.print(" | Valve: "); Serial.print(valvePercent); Serial.print("%");
            Serial.print(" | Temperature: "); Serial.println(temperature);

          publishStatus(toJson(isLocked, mode, windowOpen, valvePercent, temperature));

        });
    } else {
    }

    return true;
}


void Thermostat::setLock(boolean locked) {
   if (!writeChar) {
        Serial.println("Characteristic not available!");
        return;
    }
    uint8_t value[2];   // <-- HIER deklarieren!
    value[0] = 0x80;               // Steuerbyte bleibt immer 0x80
    value[1] = locked ? 0x01 : 0x00;  // Lock-/Unlock-Flag

    if (writeChar->canWrite()) {
        writeChar->writeValue(value, 2, true);
    }
}

void Thermostat::setTemperature(float temperature) {
  Serial.println("Starte senden an die Heizung");
  if (!writeChar) {
    Serial.println("Pröööblem");
      return;
    }

   // 1) Temperatur in Format umwandeln (°C * 2 → Hex)
  uint8_t hexTemp = (uint8_t)(temperature * 2);  // 20.0°C → 40
  uint8_t payload[2];

  payload[0] = 0x41;       // "Set Temperature" command prefix
  payload[1] = hexTemp;    // Temperaturwert * 2

  writeChar->writeValue(payload, 2, true);
  delay(3000);
  Serial.println("gesendet an die Heizung!");
}

void Thermostat::publishStatus(const char* message) {
    if (mqttClient && mqttClient->connected()) {
        mqttClient->publish("test/topic", message, true);
    }
}

String Thermostat::getThermostatID() {
        return String(deviceConfig.id);  // ID als String zurückgeben
}


const char* Thermostat::toJson(bool isLocked, String mode, bool windowOpen, uint8_t valvePercent, float temperature) {
    // Statischer Puffer, bleibt nach Funktionsende erhalten
    static char json[128];

    snprintf(json, sizeof(json),
             "{\"locked\": %s, \"mode\": \"%s\", \"windowOpen\": %s, \"valvePercent\": %u, \"temperature\": %.1f}",
             isLocked ? "true" : "false",
             mode.c_str(),
             windowOpen ? "true" : "false",
             valvePercent,
             temperature);

    return json;
}

void Thermostat::handleCommand(char* topic, byte* payload, unsigned int length) {

    String command = extractCommand(topic);  // Command aus Topic extrahieren
  	
    if (command == "setTemperature") {
        float temp = String((char*)payload, length).toFloat();
        setTemperature(temp);
    } else if (command == "getStatus") {
        //publishStatus();
    } else if (command == "setLock") {
        String value((char*)payload, length);
        value.trim();
        // you can use true/false or 1/0 for the payload
        bool newStatus = value.equalsIgnoreCase("true") || value == "1";
        setLock(newStatus);
    } else if (command == "boost") {
        //boostMode();
    }
}

String Thermostat::extractCommand(const char* topic) {
    String t(topic);
    int lastSlash = t.lastIndexOf('/');                     // letzter Slash
    if (lastSlash == -1) return "";                        // ungültiges Format
    String command = t.substring(lastSlash + 1);           // alles nach letztem Slash
    return command;
}
