#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>
#include <stdint.h>

class Timer
{
public:
    explicit Timer(uint32_t durationSeconds);

    void Start(uint32_t currentTimeMs);
    uint32_t Update(uint32_t currentTimeMs);
    void Stop(uint32_t currentTimeMs);
    bool IsRunning() const;
    bool IsFinished() const;

private:
    uint32_t durationMs;
    uint32_t accumulatedMs;
    uint32_t totalElapsedMs;
    uint32_t startTime;
    bool isRunning;
};

String formatTime(uint32_t ms);

#endif // TIMER_H