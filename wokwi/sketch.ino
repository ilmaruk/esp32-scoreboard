#include "Timer.h"

Timer* t = new Timer(65);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32-S3!");

  t->Start(millis());
}

void loop() {
  uint32_t remaining = t->Update(millis());
  Serial.print("Remaining: ");
  Serial.println(formatTime(remaining));
  delay(10); // this speeds up the simulation
}
