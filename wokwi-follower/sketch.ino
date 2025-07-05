#include <WiFi.h>
#include "time.h"
#include "sntp.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "config.h"
#include "Timer.h"
#include "commands.h"
#include "Score.h"

WiFiClient espClient;
PubSubClient client(espClient);

Timer* t = new Timer(TIMER_DURATION_SEC);

Score* s = new Score();

bool ready = false;

void reconnect_to_wifi(String ssid, String passphrase, uint8_t channel) {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Reconnecting to WiFi");
  WiFi.begin(ssid, passphrase, channel);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void reconnect_to_mqtt(PubSubClient* client) {
  // Loop until we're reconnected
  while (!client->connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client->connect(MQTT_CLIENT_NAME)) {
      Serial.println("connected");
      // Subscribe
      client->subscribe(MQTT_TOPIC, 1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client->state());
      Serial.println(" try again in 1 second");
      delay(1000);
    }
  }
}

uint64_t now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void update(Timer* t, Score *s) {
  if (!ready) return;

  static String previous;

  char buffer[15];
  snprintf(buffer, sizeof(buffer), "[%s] %2d-%2d", formatTime(t->Update(now())), s->GetHomeScore(), s->GetAwayScore());
  String current = String(buffer);
  if (current != previous) {
    Serial.println(current);
    previous = current;
  }
}

void handle_mqtt_message(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);

  JsonDocument doc;
  deserializeJson(doc, message);
  command_t cmd = doc["command"];
  uint64_t timestamp = doc["time"];

  switch (cmd) {
    case COMMAND_START_TIMER:
      t->Start(timestamp);
      break;
    case COMMAND_STOP_TIMER:
      t->Stop(timestamp);
      break;
    case COMMAND_HOME_SCORE:
    case COMMAND_AWAY_SCORE:
    case COMMAND_HOME_ADJUST:
    case COMMAND_AWAY_ADJUST:
      uint8_t val;
      val = doc["home"];
      s->SetHomeScore(val);
      val = doc["away"];
      s->SetAwayScore(val);
      break;
    default:
      Serial.print("Unhandled command: ");
      Serial.println(cmd);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Welcome to ESP32-Scoreboard!");

  WiFi.mode(WIFI_STA);

  sntp_set_time_sync_notification_cb(time_available);
  sntp_servermode_dhcp(1);    // (optional)
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2);

  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(handle_mqtt_message);
}

void time_available(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
  ready = true;
}

void loop() {
  reconnect_to_wifi(WIFI_SSID, WIFI_PASSPHRASE, WIFI_CHANNEL);
  if (!client.connected()) {
    reconnect_to_mqtt(&client);
  }
  client.loop();

  // Update the display
  update(t, s);

  delay(100);
}
