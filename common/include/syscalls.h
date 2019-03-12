#pragma once

#define SYS_EXIT            1
#define SYS_DEBUG_STR       2
#define SYS_SET_TID_ADDRESS 3
#define SYS_SET_THREAD_AREA 4
#define SYS_GET_PTHREAD     5
#define SYS_READV           6
#define SYS_WRITEV          7
#define SYS_GETPID          8
#define SYS_GETTID          9
#define SYS_BRK             10
#define SYS_GETCWD          11
#define SYS_OPEN            12
#define SYS_CLOSE           13
#define SYS_READ            14
#define SYS_WRITE           15
#define SYS_MMAP            16
#define SYS_MMAP2           17
#define SYS_MPROTECT        18
#define SYS_GETDENTS        19
#define SYS_FSTAT           20
#define SYS_MUNMAP          21
#define SYS_RT_SIGPROCMASK  22

#define SYS_GET_FB_COUNT    64
#define SYS_OPEN_FB         65
#define SYS_OPEN_DEFAULT_FB 66
#define SYS_CLOSE_FB        67
#define SYS_GET_MODE_COUNT  68
#define SYS_GET_MODE_INFO   69
#define SYS_SET_MODE        70

#define SYS_INDEV_GET_COUNT 80
#define SYS_INDEV_LIST      81
#define SYS_INDEV_OPEN      82
#define SYS_INDEV_CLOSE     83
#define SYS_INDEV_GET_TYPE  84
#define SYS_INDEV_GET_EVENT 85

#define SYS_THREAD_CREATE   96
#define SYS_THREAD_DELETE   97
#define SYS_THREAD_RESUME   98
#define SYS_THREAD_SUSPEND  99
#define SYS_THREAD_SLEEP    100
#define SYS_THREAD_WAIT     101
#define SYS_THREAD_ABORT    102

#define SYS_NOT_IMPL	    0x80000000
