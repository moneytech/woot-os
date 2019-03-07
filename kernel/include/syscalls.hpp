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
    static long sys_getcwd(uintptr_t *args);
    static long sys_open(uintptr_t *args);
    static long sys_close(uintptr_t *args);
    static long sys_read(uintptr_t *args);
    static long sys_write(uintptr_t *args);
    static long sys_mmap(uintptr_t *args);
    static long sys_mmap2(uintptr_t *args);
    static long sys_mprotect(uintptr_t *args);
    static long sys_getdents(uintptr_t *args);
    static long sys_fstat(uintptr_t *args);

    static long sys_get_fb_count(uintptr_t *args);
    static long sys_open_fb(uintptr_t *args);
    static long sys_open_default_fb(uintptr_t *args);
    static long sys_close_fb(uintptr_t *args);
    static long sys_get_mode_count(uintptr_t *args);
    static long sys_get_mode_info(uintptr_t *args);
    static long sys_set_mode(uintptr_t *args);
public:
    static void Initialize();
    static void Cleanup();
};
