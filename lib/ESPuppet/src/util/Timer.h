#include "EventBroadcaster.h"

class Timer;
class TimerEvent
{
public:
    TimerEvent(Timer * timer) : timer(timer) {}
    Timer * timer;
};

class Timer : public EventBroadcaster<TimerEvent> {
public: 
    Timer(long durationMs, bool loop = false) : isRunning(false), timeAtStart(0), intervalMs(durationMs), doLoop(loop)
    {}

    ~Timer() {}

    bool isRunning;
    long timeAtStart;
    long intervalMs;
    bool doLoop;

    void set(long durationMs, bool loop = false)
    {
        intervalMs = durationMs;
        doLoop = loop;
    }

    void start()
    {
        timeAtStart = millis();
        isRunning = true;
    }

    void stop()
    {
        isRunning = false;
    }

    void update()
    {
        if(!isRunning) return;
        if(millis() - timeAtStart >= intervalMs)
        {
            isRunning = false;
            sendEvent(TimerEvent(this));
            if (doLoop) start();
        }
    }
};