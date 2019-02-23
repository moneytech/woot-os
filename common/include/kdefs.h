#pragma once

#define O_RDONLY    00
#define O_WRONLY    01
#define O_RDWR      02
#define O_ACCMODE   03
#define O_CREAT     0100
#define O_EXCL      0200
#define O_NOCTTY    0400
#define O_TRUNC     01000
#define O_APPEND    02000
#define O_NONBLOCK  04000
#define O_DSYNC     010000
#define O_SYNC      04010000
#define O_RSYNC     04010000
#define O_DIRECTORY 0200000
#define O_NOFOLLOW  0400000
#define O_CLOEXEC   02000000
#define O_ASYNC     020000
#define O_DIRECT    040000
#define O_LARGEFILE 0100000
#define O_NOATIME   01000000
#define O_PATH      010000000
#define O_TMPFILE   020200000
#define O_NDELAY    O_NONBLOCK

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define S_IFTYPE 0xF000 // type mask
#define S_IFSOCK 0xC000 // socket
#define S_IFLNK  0xA000 // symbolic link
#define S_IFREG  0x8000 // regular file
#define S_IFBLK  0x6000 // block device
#define S_IFDIR  0x4000 // directory
#define S_IFCHR  0x2000 // character device
#define S_IFIFO  0x1000 // fifo

#define S_ISUID  0x0800 // Set process User ID
#define S_ISGID  0x0400 // Set process Group ID
#define S_ISVTX  0x0200 // sticky bit

#define S_IRUSR  0x0100 // user read
#define S_IWUSR  0x0080 // user write
#define S_IXUSR  0x0040 // user execute
#define S_IRGRP  0x0020 // group read
#define S_IWGRP  0x0010 // group write
#define S_IXGRP  0x0008 // group execute
#define S_IROTH  0x0004 // others read
#define S_IWOTH  0x0002 // others write
#define S_IXOTH  0x0001 // others execute

#define S_ISDIR(mode)	(((mode) & S_IFTYPE) == S_IFDIR)
#define S_ISLINK(mode)	(((mode) & S_IFTYPE) == S_IFLNK)
#define S_ISBLK(mode)	(((mode) & S_IFTYPE) == S_IFBLK)
#define S_ISSOCK(mode)	(((mode) & S_IFTYPE) == S_IFSOCK)
#define S_ISREG(mode)	(((mode) & S_IFTYPE) == S_IFREG)
#define S_ISCHAR(mode)	(((mode) & S_IFTYPE) == S_IFCHR)
#define S_ISFIFO(mode)	(((mode) & S_IFTYPE) == S_IFIFO)

