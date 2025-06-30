#include <Arduino.h>
#include <Timer.h>

Timer timer(5); // 5-second timer

void setup()
{
    Serial.begin(115200);
    timer.Start(millis());
}

void loop()
{
    uint32_t remaining = timer.Update(millis());
    Serial.print("Time remaining: ");
    Serial.println(remaining);
    delay(1000);
}