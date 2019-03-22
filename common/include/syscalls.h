#pragma once

#define SYS_EXIT                1
#define SYS_EXIT_GROUP          2
#define SYS_DEBUG_STR           3
#define SYS_SET_TID_ADDRESS     4
#define SYS_SET_THREAD_AREA     5
#define SYS_GET_PTHREAD         6
#define SYS_READV               7
#define SYS_WRITEV              8
#define SYS_GETPID              9
#define SYS_GETTID              10
#define SYS_BRK                 11
#define SYS_GETCWD              12
#define SYS_OPEN                13
#define SYS_CLOSE               14
#define SYS_READ                15
#define SYS_WRITE               16
#define SYS_MMAP                17
#define SYS_MMAP2               18
#define SYS_MPROTECT            19
#define SYS_GETDENTS            20
#define SYS_FSTAT               21
#define SYS_MUNMAP              22
#define SYS_RT_SIGPROCMASK      23
#define SYS_LSEEK               24

// display
#define SYS_FB_GET_COUNT        64
#define SYS_FB_OPEN             65
#define SYS_FB_OPEN_DEFAULT     66
#define SYS_FB_CLOSE            67
#define SYS_FB_GET_MODE_COUNT   68
#define SYS_FB_GET_MODE_INFO    69
#define SYS_FB_SET_MODE         70
#define SYS_FB_MAP_PIXELS       71
#define SYS_FB_GET_CURRENT_MODE 72

// input
#define SYS_INDEV_GET_COUNT     80
#define SYS_INDEV_LIST          81
#define SYS_INDEV_OPEN          82
#define SYS_INDEV_CLOSE         83
#define SYS_INDEV_GET_TYPE      84
#define SYS_INDEV_GET_NAME      85
#define SYS_INDEV_GET_EVENT     86

// thread control
#define SYS_THREAD_CREATE       96
#define SYS_THREAD_DELETE       97
#define SYS_THREAD_RESUME       98
#define SYS_THREAD_SUSPEND      99
#define SYS_THREAD_SLEEP        100
#define SYS_THREAD_WAIT         101
#define SYS_THREAD_ABORT        102
#define SYS_THREAD_DAEMONIZE    103
#define SYS_THREAD_GET_ID       104

// IPC
#define SYS_IPC_SEND_MESSAGE    112
#define SYS_IPC_GET_MESSAGE     113
#define SYS_IPC_CREATE_SHMEM    114
#define SYS_IPC_OPEN_SHMEM      115
#define SYS_IPC_CLOSE_SHMEM     116
#define SYS_IPC_GET_SHMEM_SIZE  117
#define SYS_IPC_MAP_SHMEM       118
#define SYS_IPC_UNMAP_SHMEM     119

// process control
#define SYS_PROCESS_CREATE      128
#define SYS_PROCESS_DELETE      129
#define SYS_PROCESS_WAIT        130
#define SYS_PROCESS_ABORT       131

// signals
#define SYS_SIGNAL_GET_HANDLER  144
#define SYS_SIGNAL_SET_HANDLER  145
#define SYS_SIGNAL_IS_ENABLED   146
#define SYS_SIGNAL_ENABLE       147
#define SYS_SIGNAL_DISABLE      148
#define SYS_SIGNAL_RAISE        149
#define SYS_SIGNAL_RETURN       150
#define SYS_SIGNAL_GET_CURRENT  151

#define SYS_NOT_IMPL            0x80000000
