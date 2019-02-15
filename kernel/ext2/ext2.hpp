#pragma once

#include <filesystem.hpp>
#include <filesystemtype.hpp>

#define EXT2_SUPER_MAGIC 0xEF53

#define EXT2_BAD_INO         1
#define EXT2_ROOT_INO        2
#define EXT2_ACL_IDX_INO     3
#define EXT2_ACL_DATA_INO    4
#define EXT2_BOOT_LOADER_INO 5
#define EXT2_UNDEL_DIR_INO   6

#define EXT2_VALID_FS 1
#define EXT2_ERROR_FS 2

#define EXT2_ERRORS_CONTINUE 1
#define EXT2_ERRORS_RO       2
#define EXT2_ERRORS_PANIC    3

#define EXT2_FEATURE_COMPAT_DIR_PREALLOC  0x0001
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES 0x0002
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL   0x0004
#define EXT2_FEATURE_COMPAT_EXT_ATTR      0x0008
#define EXT2_FEATURE_COMPAT_RESIZE_INO    0x0010
#define EXT2_FEATURE_COMPAT_DIR_INDEX     0x0020

#define EXT2_FEATURE_INCOMPAT_COMPRESSION   0x0001
#define EXT2_FEATURE_INCOMPAT_FILETYPE      0x0002
#define EXT3_FEATURE_INCOMPAT_RECOVER       0x0004
#define EXT3_FEATURE_INCOMPAT_JOURNAL_DEV   0x0008
#define EXT2_FEATURE_INCOMPAT_META_BG       0x0010
#define EXTx_FEATURE_INCOMPAT_FUTURE        0xFFE0

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER 0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE   0x0002
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR    0x0004
#define EXTx_FEATURE_RO_COMPAT_FUTURE       0xFFF8

#define EXT2_S_IFTYPE 0xF000 // type mask
#define EXT2_S_IFSOCK 0xC000 // socket
#define EXT2_S_IFLNK  0xA000 // symbolic link
#define EXT2_S_IFREG  0x8000 // regular file
#define EXT2_S_IFBLK  0x6000 // block device
#define EXT2_S_IFDIR  0x4000 // directory
#define EXT2_S_IFCHR  0x2000 // character device
#define EXT2_S_IFIFO  0x1000 // fifo

#define EXT2_S_ISUID  0x0800 // Set process User ID
#define EXT2_S_ISGID  0x0400 // Set process Group ID
#define EXT2_S_ISVTX  0x0200 // sticky bit

#define EXT2_S_IRUSR  0x0100 // user read
#define EXT2_S_IWUSR  0x0080 // user write
#define EXT2_S_IXUSR  0x0040 // user execute
#define EXT2_S_IRGRP  0x0020 // group read
#define EXT2_S_IWGRP  0x0010 // group write
#define EXT2_S_IXGRP  0x0008 // group execute
#define EXT2_S_IROTH  0x0004 // others read
#define EXT2_S_IWOTH  0x0002 // others write
#define EXT2_S_IXOTH  0x0001 // others execute

#define EXT2_S_ISDIR(mode)	(((mode) & EXT2_S_IFTYPE) == EXT2_S_IFDIR)
#define EXT2_S_ISLINK(mode)	(((mode) & EXT2_S_IFTYPE) == EXT2_S_IFLNK)
#define EXT2_S_ISBLK(mode)	(((mode) & EXT2_S_IFTYPE) == EXT2_S_IFBLK)
#define EXT2_S_ISSOCK(mode)	(((mode) & EXT2_S_IFTYPE) == EXT2_S_IFSOCK)
#define EXT2_S_ISREG(mode)	(((mode) & EXT2_S_IFTYPE) == EXT2_S_IFREG)
#define EXT2_S_ISCHAR(mode)	(((mode) & EXT2_S_IFTYPE) == EXT2_S_IFCHR)
#define EXT2_S_ISFIFO(mode)	(((mode) & EXT2_S_IFTYPE) == EXT2_S_IFIFO)

#define EXT2_FT_UNKNOWN     0   // Unknown File Type
#define EXT2_FT_REG_FILE    1   // Regular File
#define EXT2_FT_DIR         2   // Directory File
#define EXT2_FT_CHRDEV      3   // Character Device
#define EXT2_FT_BLKDEV      4   // Block Device
#define EXT2_FT_FIFO        5   // Buffer File
#define EXT2_FT_SOCK        6   // Socket File
#define EXT2_FT_SYMLINK     7   // Symbolic Link

class EXT2FileSystemType : public FileSystemType
{
public:
    EXT2FileSystemType(bool autoRegister);
    virtual int Detect(Volume *vol);
};

class EXT2FileSystem : public FileSystem
{

};
