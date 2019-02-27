#include <random.hpp>

uint Random::rsx = 0x1BADB002;
uint Random::rsy = 0xBAADF00D;
uint Random::rsz = 0xB16B00B2;
uint Random::rsw = 0x10121337;

const uint MaxValue = 0xFFFFFFFFu;

void Random::SetSeed(uint seed)
{
    // FIXME: needs lock
    rsx = 0x1BADB002 ^ seed;
    rsy = 0xBAADF00D ^ seed;
    rsz = 0xB16B00B2 ^ seed;
    rsw = 0x10121337 ^ seed;
}

uint Random::GetValue()
{
    // FIXME: needs lock
    int t = rsx ^ (rsx << 11);
    rsx = rsy; rsy = rsz; rsz = rsw;
    rsw = rsw ^ (rsw >> 19) ^ t ^ (t >> 8);
    return rsw;
}
