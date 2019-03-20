#pragma once

#include <list.hpp>
#include <types.hpp>
#include <vector.hpp>

class Process;

class SharedMem
{
    Vector<uintptr_t> frames;
public:
    struct Mapping
    {
        Process *Owner;
        uintptr_t VA;
    };

    SharedMem(size_t size);
    ~SharedMem();
};
