#include <WiFi.h>
#include "time.h"
#include "sntp.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <IRremote.hpp>

#include "config.h"
#include "Timer.h"
#include "IRProcessor.h"
#include "commands.h"
#include "Score.h"

WiFiClient espClient;
PubSubClient client(espClient);

Timer* t = new Timer(TIMER_DURATION_SEC);

IRProcessor::CommandMap wokwiRemoteCommandMap = {
    {168, COMMAND_OPERATE_TIMER},
    {224, COMMAND_HOME_SCORE},
    {144, COMMAND_AWAY_SCORE},
    {104, COMMAND_HOME_ADJUST},
    {176, COMMAND_AWAY_ADJUST},
};
IRProcessor* p = new IRProcessor(IrReceiver, wokwiRemoteCommandMap);

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
  static String previous;

  String current;
  if (!ready) {
    current = "Initialisation";
  } else if(!t->HasStarted()) {
    struct tm timeinfo;
    if(getLocalTime(&timeinfo)) {
      char buffer[15];
      strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
      current = String(buffer);
    }
  } else {
    char buffer[15];
    snprintf(buffer, sizeof(buffer), "[%s] %2d-%2d", formatTime(t->Update(now())), s->GetHomeScore(), s->GetAwayScore());
    current = String(buffer);
  }

  if (current != previous) {
    Serial.println(current);
    previous = current;
  }
}

void publish_event(PubSubClient* client, uint64_t timestamp, command_t cmd, Score* s) {
  reconnect_to_mqtt(client);

  JsonDocument doc;
  doc["command"] = cmd;
  doc["time"] = timestamp;
  doc["home"] = s->GetHomeScore();
  doc["away"] = s->GetAwayScore();

  String jsonString;
  serializeJson(doc, jsonString);

  client->publish(MQTT_TOPIC, jsonString.c_str());
}

void setup() {
  Serial.begin(115200);
  Serial.println("Welcome to ESP32-Scoreboard!");

  WiFi.mode(WIFI_STA);

  sntp_set_time_sync_notification_cb(time_available);
  sntp_servermode_dhcp(1);    // (optional)
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2);

  client.setServer(MQTT_SERVER, 1883);

  IrReceiver.begin(0);
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

  // Process IR commands
  int16_t cmd;
  if ((cmd = p->Process()) != COMMAND_NONE) {
    uint64_t timestamp = now();
    switch (cmd) {
      case COMMAND_OPERATE_TIMER:
      case COMMAND_START_TIMER:
      case COMMAND_STOP_TIMER:
        if (t->IsRunning()) {
          t->Stop(timestamp);
          publish_event(&client, timestamp, COMMAND_STOP_TIMER, s);
        } else {
          t->Start(timestamp);
          publish_event(&client, timestamp, COMMAND_START_TIMER, s);
        }
        break;
      case COMMAND_HOME_SCORE:
        if (!t->IsRunning()) break;
        s->HomeScore();
        publish_event(&client, timestamp, cmd, s);
        break;
      case COMMAND_AWAY_SCORE:
        if (!t->IsRunning()) break;
        s->AwayScore();
        publish_event(&client, timestamp, cmd, s);
        break;
      case COMMAND_HOME_ADJUST:
        if (!t->IsRunning()) break;
        s->HomeAdjust();
        publish_event(&client, timestamp, cmd, s);
        break;
      case COMMAND_AWAY_ADJUST:
        if (!t->IsRunning()) break;
        s->AwayAdjust();
        publish_event(&client, timestamp, cmd, s);
        break;
      default:
        Serial.print("Unhandled command: ");
        Serial.println(cmd);
    }
  }

  // Update the display
  update(t, s);

  delay(100);
}
