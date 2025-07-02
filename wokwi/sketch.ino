#include <IRremote.hpp>
#include "Timer.h"
#include "IRProcessor.h"
#include "commands.h"

Timer* t = new Timer(65);
String prevTime;

IRProcessor::CommandMap wokwiRemoteCommandMap = {
    {168, COMMAND_OPERATE_TIMER},
    {224, COMMAND_HOME_SCORE},
    {144, COMMAND_AWAY_SCORE},
};
IRProcessor* p = new IRProcessor(IrReceiver, wokwiRemoteCommandMap);

void setup() {
  Serial.begin(115200);
  Serial.println("Welcome to ESP32 Scoreboard");

  IrReceiver.begin(0);
}

void loop() {
  // Update the timer
  String currTime = formatTime(t->Update(millis()));
  if (currTime != prevTime) {
    Serial.print("Remaining: ");
    Serial.println(currTime);
    prevTime = currTime;
  }

  // Process IR commands
  int16_t cmd;
  if ((cmd = p->Process()) != COMMAND_NONE) {
    Serial.print("Command: ");
    Serial.println(cmd);
    switch (cmd) {
      case COMMAND_OPERATE_TIMER:
        if (t->IsRunning()) {
          t->Stop(millis());
        } else {
          t->Start(millis());
        }
    }
  }

  delay(100);
}
