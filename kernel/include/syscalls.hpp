#pragma once

#include <types.hpp>

class SysCalls
{
    static long (*handlers[])(uintptr_t *args);
    static long handler(uintptr_t *args);

    static long sys_exit(uintptr_t *args);
    static long sys_debug_str(uintptr_t *args);
    static long sys_set_tid_address(uintptr_t *args);
    static long sys_set_thread_area(uintptr_t *args);
    static long sys_get_pthread(uintptr_t *args);
    static long sys_readv(uintptr_t *args);
    static long sys_writev(uintptr_t *args);
    static long sys_getpid(uintptr_t *args);
    static long sys_gettid(uintptr_t *args);
    static long sys_brk(uintptr_t *args);
public:
    static void Initialize();
    static void Cleanup();
};
