#include <IRremote.hpp>
#include "Timer.h"
#include "IRProcessor.h"
#include "commands.h"
#include "Score.h"

Timer* t = new Timer(10 * 60);

IRProcessor::CommandMap wokwiRemoteCommandMap = {
    {168, COMMAND_OPERATE_TIMER},
    {224, COMMAND_HOME_SCORE},
    {144, COMMAND_AWAY_SCORE},
    {104, COMMAND_HOME_ADJUST},
    {176, COMMAND_AWAY_ADJUST},
};
IRProcessor* p = new IRProcessor(IrReceiver, wokwiRemoteCommandMap);

Score* s = new Score();

void setup() {
  Serial.begin(115200);
  Serial.println("Welcome to ESP32-Scoreboard!");

  IrReceiver.begin(0);
}

void update(Timer* t, Score *s) {
  static String previous;

  char buffer[15];
  snprintf(buffer, sizeof(buffer), "[%s] %2d-%2d", formatTime(t->Update(millis())), s->GetHomeScore(), s->GetAwayScore());
  String current = String(buffer);
  if (current != previous) {
    Serial.println(current);
    previous = current;
  }
}

void loop() {
  // Process IR commands
  int16_t cmd;
  if ((cmd = p->Process()) != COMMAND_NONE) {
    Serial.print("Command: ");
    Serial.println(cmd);
    switch (cmd) {
      case COMMAND_OPERATE_TIMER:
      case COMMAND_START_TIMER:
      case COMMAND_STOP_TIMER:
        if (t->IsRunning()) {
          t->Stop(millis());
        } else {
          t->Start(millis());
        }
        break;
      case COMMAND_HOME_SCORE:
        if (!t->IsRunning()) break;
        s->HomeScore();
        break;
      case COMMAND_AWAY_SCORE:
        if (!t->IsRunning()) break;
        s->AwayScore();
        break;
      case COMMAND_HOME_ADJUST:
        if (!t->IsRunning()) break;
        s->HomeAdjust();
        break;
      case COMMAND_AWAY_ADJUST:
        if (!t->IsRunning()) break;
        s->AwayAdjust();
        break;
    }
  }

  // Update the display
  update(t, s);

  delay(100);
}
