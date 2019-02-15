#include <cmos.hpp>
#include <cpu.hpp>
#include <errno.h>
#include <irqs.hpp>
#include <thread.hpp>
#include <time.hpp>

uint64_t Time::systemTicks = 0;
uint64_t Time::systemTickFreq = 1;
bool Time::isFakeTick = false;
Ints::Handler Time::tickHandler = { nullptr, Time::Tick, nullptr };

static uint8_t bcdToDec(uint8_t bcd)
{
    return (bcd & 0x0F) + (bcd >> 4) * 10;
}

bool Time::Tick(Ints::State *state, void *context)
{
    bool isIRQ = CMOS::Read(0x0C) & 0x80;
    Thread::Switch(state, Thread::GetNext(isIRQ && !isFakeTick));
    isFakeTick = false;
    if(isIRQ)
        ++systemTicks;
    return true;
}

void Time::Initialize()
{
    IRQs::RegisterHandler(CMOS::GetIRQ(), &tickHandler);
    CMOS::SetTimerDivider(9);
    systemTickFreq = Time::GetTickFrequency();
}

void Time::StartSystemTimer()
{
    CMOS::EnableTimer();
    IRQs::Enable(CMOS::GetIRQ());
}

void Time::GetDateTime(Time::DateTime *dt)
{
    if(!dt) return;

    uint8_t s1, m1, h1, D1, M1, Y1;
    uint8_t s2, m2, h2, D2, M2, Y2;
    for(;;)
    {
        bool cs = cpuDisableInterrupts();
        s1 = CMOS::Read(0);
        m1 = CMOS::Read(2);
        h1 = CMOS::Read(4);
        D1 = CMOS::Read(7);
        M1 = CMOS::Read(8);
        Y1 = CMOS::Read(9);

        s2 = CMOS::Read(0);
        m2 = CMOS::Read(2);
        h2 = CMOS::Read(4);
        D2 = CMOS::Read(7);
        M2 = CMOS::Read(8);
        Y2 = CMOS::Read(9);
        cpuRestoreInterrupts(cs);

        if(s1 == s2 && m1 == m2 && h1 == h2 && D1 == D2 && M1 == M2 && Y1 == Y2)
            break;
    }

    dt->Year = 2000 + bcdToDec(Y1);
    dt->Month = bcdToDec(M1);
    dt->Day = bcdToDec(D1);
    dt->Hour = bcdToDec(h1);
    dt->Minute = bcdToDec(m1);
    dt->Second = bcdToDec(s1);
    dt->Millisecond = 0;
}

void Time::UnixToDateTime(time_t t, Time::DateTime *date)
{
    if(!date) return;
    if(t < 0) t = 0; // negative time not supported
    date->Millisecond = 0;
    date->Second = t % 60;
    t /= 60;
    date->Minute = t % 60;
    t /= 60;
    date->Hour = t % 24;
    t /= 24;

    time_t a = (4 * t + 102032) / 146097 + 15;
    time_t b = t + 2442113 + a - (a / 4);
    time_t c = (20 * b - 2442) / 7305;
    time_t d = b - 365 * c - (c / 4);
    time_t e = d * 1000 / 30601;
    time_t f = d - e * 30 - e * 601 / 1000;

    if(e <= 13)
    {
        c -= 4716;
        e -= 1;
    }
    else
    {
        c -= 4715;
        e -= 13;
    }

    date->Year = c;
    date->Month = e;
    date->Day = f;
}

void Time::FracUnixToDateTime(double t, Time::DateTime *date)
{
    if(!date) return;
    time_t intTime = (time_t)t;
    Time::UnixToDateTime(intTime, date);
    date->Millisecond = (t - intTime) * 1000;
}

time_t Time::DateTimeToUnix(const Time::DateTime *date)
{
    if(!date) return 0;

    time_t t;
    time_t y = date->Year;
    time_t m = date->Month;
    time_t d = date->Day;

    if(m <= 2)
    {
        m += 12;
        y -= 1;
    }

    t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
    t += (30 * m) + (3 * (m + 1) / 5) + d;
    t -= 719561;
    t *= 86400;
    t += (3600 * date->Hour) + (60 * date->Minute) + date->Second;

    return t;
}

uint64_t Time::GetTickFrequency()
{
    return CMOS::GetTimerFrequency();
}

uint64_t Time::GetTickCount()
{
    bool cs = cpuDisableInterrupts();
    uint64_t ticks = systemTicks;
    cpuRestoreInterrupts(cs);
    return ticks;
}

double Time::GetSystemUpTime()
{
    return (double)GetTickCount() / (double)systemTickFreq;
}

void Time::FakeTick()
{
    bool ints = cpuDisableInterrupts();
    isFakeTick = true;
    cpuINT(IRQS_BASE + 8);
    cpuRestoreInterrupts(ints);
}

uint Time::Sleep(uint millis, bool interruptible)
{
    Thread *ct = Thread::GetCurrent();
    if(!ct) return 0;
    return ct->Sleep(millis, interruptible);
}

time_t Time::GetTime()
{
    Time::DateTime dt;
    Time::GetDateTime(&dt);
    return Time::DateTimeToUnix(&dt);
}

time_t Time::GetTimeOfDay()
{
    Time::DateTime dt;
    Time::GetDateTime(&dt);
    dt.Year = 0;
    dt.Month = 0;
    dt.Day = 0;
    return Time::DateTimeToUnix(&dt);
}
