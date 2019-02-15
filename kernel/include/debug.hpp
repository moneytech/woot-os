#pragma once

#include <stream.hpp>

extern Stream *DebugStream;

#define DEBUG DebugStream->WriteFmt

class Debug
{
public:
    static void BufferDump(void *ptr, size_t n);
};
