#pragma once

// Types that need to be shared between kernel and userland

typedef int pid_t;
typedef int time_t;
typedef int uid_t;
typedef int gid_t;
typedef int mode_t;
typedef long long ino_t;
typedef long long off_t;

typedef unsigned long long dev_t;
typedef unsigned int nlink_t;
typedef unsigned int blksize_t;
typedef unsigned long long blkcnt_t;

struct timespec
{
    time_t tv_sec;
    long tv_nsec;
};

struct dirent
{
	ino_t d_ino;
	off_t d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[];
};

struct stat
{
    dev_t st_dev;
    int __st_dev_padding;
    long __st_ino_truncated;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    int __st_rdev_padding;
    off_t st_size;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    ino_t st_ino;
};

struct vidModeInfo
{
    int Width, Height;
    int BitsPerPixel;
    int RefreshRate;
    int Pitch;
    int Flags;
    int AlphaBits, RedBits, GreenBits, BlueBits;
    int AlphaShift, RedShift, GreenShift, BlueShift;
};

#include <msgnums.h>

struct ipcMessage
{
    int Number, Flags, ID, Source;
    unsigned char Data[MSG_PAYLOAD_SIZE];
};
