#pragma once

#include <ints.hpp>
#include <ktypes.h>
#include <types.hpp>

class Time
{
    static uint64_t systemTicks;
    static uint64_t systemTickFreq;
public:
    static bool isFakeTick; // TODO: make this private
private:
    static Ints::Handler tickHandler;
    static bool Tick(Ints::State *state, void *context);
public:

    struct DateTime
    {
        int Year;
        int Month;
        int Day;
        int Hour;
        int Minute;
        int Second;
        int Millisecond;
    };

    static void Initialize();
    static void StartSystemTimer();
    static void GetDateTime(DateTime *dt);
    static void UnixToDateTime(time_t t, DateTime *date);
    static void FracUnixToDateTime(double t, DateTime *date);
    static time_t DateTimeToUnix(const DateTime *date);
    static uint64_t GetTickFrequency();
    static uint64_t GetTickCount();
    static double GetSystemUpTime();
    static void FakeTick();
    static uint Sleep(uint millis, bool interruptible);
    static time_t GetTime();
    static time_t GetTimeOfDay();
    static uint64_t GetTimeStamp();
};
