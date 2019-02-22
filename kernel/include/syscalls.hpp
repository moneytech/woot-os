#pragma once

#include <syscall.h>
#include <types.hpp>

class SysCalls
{
    static long (*handlers[])(uintptr_t *args);
    static long handler(uintptr_t *args);

    static long sys_exit(uintptr_t *args);
    static long sys_debug_str(uintptr_t *args);
public:
    static void Initialize();
    static void Cleanup();
};
