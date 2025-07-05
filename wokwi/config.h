#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// Wi-Fi
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSPHRASE = "";
const uint8_t WIFI_CHANNEL = 6;

// MQTT
const char* MQTT_SERVER = "broker.hivemq.com";
const char* MQTT_TOPIC = "esp32/scoreboard";
const char* MQTT_CLIENT_NAME = "ESP32-Scoreboard-Leader";

// Time
// TimeZone rule for Europe/Rome including daylight adjustment rules (optional)
const char* TIME_ZONE = "CET-1CEST,M3.5.0,M10.5.0/3";
const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.nist.gov";
const long  GMT_OFFSET_SEC = 3600;
const int   DAYLIGHT_OFFSET_SEC = 3600;

// Timer
const uint64_t TIMER_DURATION_SEC = 70;

#endif // CONFIG_H