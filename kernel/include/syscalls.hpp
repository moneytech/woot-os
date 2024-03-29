#pragma once

#include <types.hpp>

extern "C" long __doSyscall(int no, ...);

class SysCalls
{
    static long (*handlers[])(uintptr_t *args);
    static long handler(uintptr_t *args);

    static long sys_exit(uintptr_t *args);
    static long sys_exit_group(uintptr_t *args);
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
    static long sys_munmap(uintptr_t *args);
    static long sys_rt_sigprocmask(uintptr_t *args);
    static long sys_lseek(uintptr_t *args);

    static long sys_fb_get_count(uintptr_t *args);
    static long sys_fb_open(uintptr_t *args);
    static long sys_fb_open_default(uintptr_t *args);
    static long sys_fb_close(uintptr_t *args);
    static long sys_fb_get_mode_count(uintptr_t *args);
    static long sys_fb_get_mode_info(uintptr_t *args);
    static long sys_fb_set_mode(uintptr_t *args);
    static long sys_fb_map_pixels(uintptr_t *args);
    static long sys_fb_get_current_mode(uintptr_t *args);

    static long sys_indev_get_count(uintptr_t *args);
    static long sys_indev_list(uintptr_t *args);
    static long sys_indev_open(uintptr_t *args);
    static long sys_indev_close(uintptr_t *args);
    static long sys_indev_get_type(uintptr_t *args);
    static long sys_indev_get_name(uintptr_t *args);
    static long sys_indev_get_event(uintptr_t *args);

    static long sys_thread_create(uintptr_t *args);
    static long sys_thread_delete(uintptr_t *args);
    static long sys_thread_resume(uintptr_t *args);
    static long sys_thread_suspend(uintptr_t *args);
    static long sys_thread_sleep(uintptr_t *args);
    static long sys_thread_wait(uintptr_t *args);
    static long sys_thread_abort(uintptr_t *args);
    static long sys_thread_daemonize(uintptr_t *args);
    static long sys_thread_get_id(uintptr_t *args);

    static long sys_ipc_send_message(uintptr_t *args);
    static long sys_ipc_get_message(uintptr_t *args);
    static long sys_ipc_create_shmem(uintptr_t *args);
    static long sys_ipc_open_shmem(uintptr_t *args);
    static long sys_ipc_close_shmem(uintptr_t *args);
    static long sys_ipc_get_shmem_size(uintptr_t *args);
    static long sys_ipc_map_shmem(uintptr_t *args);
    static long sys_ipc_unmap_shmem(uintptr_t *args);

    static long sys_process_create(uintptr_t *args);
    static long sys_process_delete(uintptr_t *args);
    static long sys_process_wait(uintptr_t *args);
    static long sys_process_abort(uintptr_t *args);

    static long sys_signal_get_handler(uintptr_t *args);
    static long sys_signal_set_handler(uintptr_t *args);
    static long sys_signal_is_enabled(uintptr_t *args);
    static long sys_signal_enable(uintptr_t *args);
    static long sys_signal_disable(uintptr_t *args);
    static long sys_signal_raise(uintptr_t *args);
    static long sys_signal_return(uintptr_t *args);
    static long sys_signal_get_current(uintptr_t *args);

public:
    static void Initialize();
    static void Cleanup();
};
