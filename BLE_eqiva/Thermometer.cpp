#include "Thermometer.h"

Thermometer::Thermometer(TheNetwork& _networkBridge, DeviceConfig* _deviceConfig, int _deviceCount): networkBridge(_networkBridge){
    deviceConfig = _deviceConfig;
    deviceCount = _deviceCount;
}

void Thermometer::setBLEScanner(NimBLEScan* _pBLEScan){
  pBLEScan = _pBLEScan;
  pBLEScan->setScanCallbacks(&scanCallbacks, false);
}

void Thermometer::scan(const unsigned long scanInterval){
  pBLEScan->start(scanInterval, false, true);
}

// publish govee values to the networkBridge
void Thermometer::publishGovee(float temperature, float humidity, int battery, int deviceIndex)
{
    // ---- Check 1: Check if deviceConfig and deviceIndex are correct ----
    if (!deviceConfig) return;
    if (deviceIndex < 0) return;

    // check if you can build the topic string with room and type
    const char* room = deviceConfig[deviceIndex].room;
    const char* type = deviceConfig[deviceIndex].deviceType;
    if (!room || !type) return;

    // just in case there is a problem
    Message msgTemp, msgHum, msgBatt;

    // ---- TEMPERATURE ----
    snprintf(msgTemp.topic, sizeof(msgTemp.topic),"home/%s/%s/temperature", room, type);
    snprintf(msgTemp.payload, sizeof(msgTemp.payload),"%.1f", temperature);
    //networkBridge.sendMqttMessage(topic, payload);
    xQueueSend(bleToMqttQueue, &msgTemp, 0);

    // ---- HUMIDITY ----
    snprintf(msgHum.topic, sizeof(msgHum.topic),"home/%s/%s/humidity", room, type);
    snprintf(msgHum.payload, sizeof(msgHum.payload),"%.1f", humidity);
    xQueueSend(bleToMqttQueue, &msgHum, 0);

    // // ---- BATTERY ----
    snprintf(msgBatt.topic, sizeof(msgBatt.topic),"home/%s/%s/battery", room, type);
    snprintf(msgBatt.payload, sizeof(msgBatt.payload),"%d", battery);
    xQueueSend(bleToMqttQueue, &msgBatt, 0);
}

// Decode Govee BLE payload into temperature, humidity, battery
// Returns true if decoding succeeded, false otherwise
bool Thermometer::decodeGovee(int len, const uint8_t* dp, int deviceIndex,float& outTemperature, float& outHumidity, int& outBattery){
    for (int i = 0; i < len; i++)
    {
        int flen = dp[i++] - 1; // field length
        int ftype = dp[i++];    // field type

        // Bounds check
        if (i + flen > len) break;

        // Example: loop over raw data bytes (optional for debugging)
        // for (int j = 0; j < flen; j++) {
        //     int c = dp[i + j];
        //     Serial.printf("[%02X]", c);
        // }

        // --- Case 1: 8-byte Govee payload ---
        if (flen == 8 && ftype == 0xFF && dp[i + 0] == 0x88 && dp[i + 1] == 0xEC)
        {
            int iTemp = int(dp[i + 3]) << 16 | int(dp[i + 4]) << 8 | int(dp[i + 5]);
            bool bNegative = iTemp & 0x800000;    // sign bit
            iTemp &= 0x7FFFF;                     // mask off sign bit

            float temperature = float(iTemp / 1000) / 10.0;
            if (bNegative) temperature = -temperature;

            float humidity = float(iTemp % 1000) / 10.0;
            int battery = int(dp[i + 6]);

            outTemperature = temperature;
            outHumidity = humidity;
            outBattery = battery;

            return true; // decoded successfully
        }

        // --- Case 2: 9-byte Govee payload ---
        if (flen == 9 && ftype == 0xFF && dp[i + 0] == 0x88 && dp[i + 1] == 0xEC)
        {
            short iTemp = short(dp[i + 4]) << 8 | short(dp[i + 3]);
            int iHumidity = int(dp[i + 6]) << 8 | int(dp[i + 5]);

            float temperature = float(iTemp) / 100.0;
            float humidity = float(iHumidity) / 100.0;
            int battery = int(dp[i + 7]);

            outTemperature = temperature;
            outHumidity = humidity;
            outBattery = battery;

            return true; // decoded successfully
        }

        i += flen - 1; // skip to next field
    }

    return false; // no valid data found
}

void Thermometer::MyScanCallbacks::onResult(const NimBLEAdvertisedDevice* advertisedDevice) {
    
     //Serial.printf("Found BLE device: %s\n", advertisedDevice->toString().c_str());
    // Danach deine MAC-Filterung

        // --- Find the device index ---
    int deviceIndex = -1;
    for (int i = 0; i < parent->deviceCount; i++) {
        if (advertisedDevice->getAddress().toString() == std::string(parent->deviceConfig[i].macAddress)) {
            deviceIndex = i;
            break;
        }
    }
    if (deviceIndex == -1) return; // unknown device

   // --- Decode sensor data ---
    const auto& payload = advertisedDevice->getPayload();
    int len = payload.size();
    const uint8_t* dp = payload.data();
    float temperature, humidity;
    int battery;

     // Assume decodeGovee can optionally return values instead of immediately publishing
    parent->decodeGovee(len, dp, deviceIndex, temperature, humidity, battery);

    // --- Store in map to filter duplicates ---
    parent->scanResults[deviceIndex] = std::make_tuple(temperature, humidity, battery);


}

void Thermometer::MyScanCallbacks::onScanEnd(const NimBLEScanResults& results, int reason) {

    Serial.printf("=== Scan Ended (reason %d) ===\n", reason);
    Serial.printf("Devices found: %d\n", (int)parent->scanResults.size());

    // Loop through all collected scanResults (duplicates automatically removed)
    for (const auto& [deviceIndex, values] : parent->scanResults) {
        float temperature, humidity;
        int battery;
        std::tie(temperature, humidity, battery) = values;

        Message msg;

        const char* room = parent->deviceConfig[deviceIndex].room;
        const char* type = parent->deviceConfig[deviceIndex].deviceType;

         Serial.printf("Device Index %d - Room: %s, Type: %s\n", deviceIndex, room, type);
        Serial.printf("Temperature: %.1f, Humidity: %.1f, Battery: %d\n",
                      temperature, humidity, battery);

        // ---- TEMPERATURE ----
        snprintf(msg.topic, sizeof(msg.topic), "home/%s/%s/temperature", room, type);
        snprintf(msg.payload, sizeof(msg.payload), "%.1f", temperature);
        xQueueSend(bleToMqttQueue, &msg, 0);

        // ---- HUMIDITY ----
        snprintf(msg.topic, sizeof(msg.topic), "home/%s/%s/humidity", room, type);
        snprintf(msg.payload, sizeof(msg.payload), "%.1f", humidity);
        xQueueSend(bleToMqttQueue, &msg, 0);

        // ---- BATTERY ----
        snprintf(msg.topic, sizeof(msg.topic), "home/%s/%s/battery", room, type);
        snprintf(msg.payload, sizeof(msg.payload), "%d", battery);
        xQueueSend(bleToMqttQueue, &msg, 0);

    }
}
