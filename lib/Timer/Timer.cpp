#include <Timer.h>

Timer::Timer(uint32_t durationSeconds)
    : durationMs(durationSeconds * 1000), isRunning(false),
      accumulatedMs(0), totalElapsedMs(0), startTime(0) {}

void Timer::Start(uint32_t currentTimeMs)
{
    if (!isRunning)
    {
        startTime = currentTimeMs;
        isRunning = true;
    }
}

uint32_t Timer::Update(uint32_t currentTimeMs)
{
    if (isRunning)
    {
        uint32_t sessionElapsed = currentTimeMs - startTime;
        totalElapsedMs = accumulatedMs + sessionElapsed;
    }
    int32_t remaining = durationMs - totalElapsedMs;
    return remaining > 0 ? remaining : 0;
}

void Timer::Stop(uint32_t currentTimeMs)
{
    if (isRunning)
    {
        uint32_t sessionElapsed = currentTimeMs - startTime;
        accumulatedMs += sessionElapsed;
        isRunning = false;
    }
}

bool Timer::IsFinished() const
{
    return totalElapsedMs >= durationMs;
}