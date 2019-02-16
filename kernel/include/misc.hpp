#pragma once

#include <types.hpp>

class Stream;

extern Stream *DebugStream;

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define clamp(_min, _max, val) (max((_min), min((_max), (val))))
#define align(val, alignment) ((alignment) * (((val) + ((alignment) - 1)) / (alignment)))
#define swap(T, a, b) { T t = (a); (a) = (b); (b) = (t); }
#define offsetof __builtin_offsetof

class Misc
{
public:    
    static unsigned long long PowULL(unsigned long long base, unsigned long long exp);
};
