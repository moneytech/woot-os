#pragma once

// Types that need to be shared between kernel and userland

typedef int pid_t;
typedef int time_t;
typedef int uid_t;
typedef int gid_t;
typedef int mode_t;
typedef long long ino_t;
typedef long long off_t;

struct dirent
{
	ino_t d_ino;
	off_t d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[];
};
