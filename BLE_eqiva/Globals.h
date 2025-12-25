// Globals.h
#pragma once
#include <Arduino.h>
#include "Message.h"

extern QueueHandle_t bleToMqttQueue;
extern QueueHandle_t mqttToBleQueue;