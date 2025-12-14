#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <Arduino.h>

// mac adress ist case sensitive!
struct DeviceConfig {
    const char* name;              // Geräte-Name
    const char* id;                // ID
    const char* room;              // Raum
    const char* deviceType;        // z.B. "thermostat", "plug", "sensor", "weather"
    const char* macAddress;        // BLE-Adresse case sensitive .. all lower cases
};

// Instanzen
extern int deviceCount;
extern int heizungsCount;
extern DeviceConfig thermoHygrometer[];
extern DeviceConfig heizungsThermostat[];
#endif