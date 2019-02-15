#pragma once

#include <filesystem.hpp>
#include <filesystemtype.hpp>
#include <inode.hpp>
#include <ktypes.h>
#include <types.hpp>

#define EXT2_SUPER_MAGIC                    0xEF53

#define EXT2_BAD_INO                        1
#define EXT2_ROOT_INO                       2
#define EXT2_ACL_IDX_INO                    3
#define EXT2_ACL_DATA_INO                   4
#define EXT2_BOOT_LOADER_INO                5
#define EXT2_UNDEL_DIR_INO                  6

#define EXT2_VALID_FS                       1
#define EXT2_ERROR_FS                       2

#define EXT2_ERRORS_CONTINUE                1
#define EXT2_ERRORS_RO                      2
#define EXT2_ERRORS_PANIC                   3

#define EXT2_FEATURE_COMPAT_DIR_PREALLOC    0x0001
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES   0x0002
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL     0x0004
#define EXT2_FEATURE_COMPAT_EXT_ATTR        0x0008
#define EXT2_FEATURE_COMPAT_RESIZE_INO      0x0010
#define EXT2_FEATURE_COMPAT_DIR_INDEX       0x0020

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

#define EXT2_S_IFTYPE                       0xF000 // type mask
#define EXT2_S_IFSOCK                       0xC000 // socket
#define EXT2_S_IFLNK                        0xA000 // symbolic link
#define EXT2_S_IFREG                        0x8000 // regular file
#define EXT2_S_IFBLK                        0x6000 // block device
#define EXT2_S_IFDIR                        0x4000 // directory
#define EXT2_S_IFCHR                        0x2000 // character device
#define EXT2_S_IFIFO                        0x1000 // fifo

#define EXT2_S_ISUID                        0x0800 // Set process User ID
#define EXT2_S_ISGID                        0x0400 // Set process Group ID
#define EXT2_S_ISVTX                        0x0200 // sticky bit

#define EXT2_S_IRUSR                        0x0100 // user read
#define EXT2_S_IWUSR                        0x0080 // user write
#define EXT2_S_IXUSR                        0x0040 // user execute
#define EXT2_S_IRGRP                        0x0020 // group read
#define EXT2_S_IWGRP                        0x0010 // group write
#define EXT2_S_IXGRP                        0x0008 // group execute
#define EXT2_S_IROTH                        0x0004 // others read
#define EXT2_S_IWOTH                        0x0002 // others write
#define EXT2_S_IXOTH                        0x0001 // others execute

#define EXT2_S_ISDIR(mode)                  (((mode) & EXT2_S_IFTYPE) == EXT2_S_IFDIR)
#define EXT2_S_ISLINK(mode)                 (((mode) & EXT2_S_IFTYPE) == EXT2_S_IFLNK)
#define EXT2_S_ISBLK(mode)                  (((mode) & EXT2_S_IFTYPE) == EXT2_S_IFBLK)
#define EXT2_S_ISSOCK(mode)                 (((mode) & EXT2_S_IFTYPE) == EXT2_S_IFSOCK)
#define EXT2_S_ISREG(mode)                  (((mode) & EXT2_S_IFTYPE) == EXT2_S_IFREG)
#define EXT2_S_ISCHAR(mode)                 (((mode) & EXT2_S_IFTYPE) == EXT2_S_IFCHR)
#define EXT2_S_ISFIFO(mode)                 (((mode) & EXT2_S_IFTYPE) == EXT2_S_IFIFO)

#define EXT2_FT_UNKNOWN                     0   // Unknown File Type
#define EXT2_FT_REG_FILE                    1   // Regular File
#define EXT2_FT_DIR                         2   // Directory File
#define EXT2_FT_CHRDEV                      3   // Character Device
#define EXT2_FT_BLKDEV                      4   // Block Device
#define EXT2_FT_FIFO                        5   // Buffer File
#define EXT2_FT_SOCK                        6   // Socket File
#define EXT2_FT_SYMLINK                     7   // Symbolic Link

class EXT2FileSystemType : public FileSystemType
{
public:
    EXT2FileSystemType(bool autoRegister);
    virtual int Detect(Volume *vol);
};

class EXT2 : public FileSystem
{
    friend class EXT2FileSystemType;
public:
#pragma pack(push, 1)
    struct SuperBlock
    {
        uint32_t s_inodes_count;
        uint32_t s_blocks_count;
        uint32_t s_r_blocks_count;
        uint32_t s_free_blocks_count;
        uint32_t s_free_inodes_count;
        uint32_t s_first_data_block;
        uint32_t s_log_block_size;
        int32_t s_log_frag_size;
        uint32_t s_blocks_per_group;
        uint32_t s_frags_per_group;
        uint32_t s_inodes_per_group;
        uint32_t s_mtime;
        uint32_t s_wtime;
        uint16_t s_mnt_count;
        uint16_t s_max_mnt_count;
        uint16_t s_magic;
        uint16_t s_state;
        uint16_t s_errors;
        uint16_t s_minor_rev_level;
        uint32_t s_last_check;
        uint32_t s_check_interval;
        uint32_t s_creator_os;
        uint32_t s_rev_level;
        uint16_t s_def_resuid;
        uint16_t s_def_resgid;

        uint32_t s_first_ino;
        uint16_t s_inode_size;
        uint16_t s_block_group_number;
        uint32_t s_feature_compat;
        uint32_t s_feature_incompat;
        uint32_t s_feature_ro_compat;
        uint8_t s_uuid[16];
        char s_volume_name[16];
        char s_last_mounted[64];
        uint32_t s_algo_bitmap;

        uint8_t s_prealloc_blocks;
        uint8_t s_prealloc_dir_blocks;
        uint16_t s_padding1;

        uint8_t s_journal_uuid[16];
        uint32_t s_journal_inum;
        uint32_t s_journal_dev;
        uint32_t s_last_orphan;

        uint32_t s_hash_seed[4];
        uint8_t s_def_hash_version;
        uint8_t s_padding2[3];

        uint32_t s_default_mount_options;
        uint32_t s_first_meta_bg;

        uint8_t unused[760];
    };

    struct BlockGroupDescriptor
    {
        uint32_t bg_block_bitmap;
        uint32_t bg_inode_bitmap;
        uint32_t bg_inode_table;
        uint16_t bg_free_blocks_count;
        uint16_t bg_free_inodes_count;
        uint16_t bg_used_dirs_count;
        uint16_t bg_pad;
        uint8_t bg_reserved[12];
    };

    struct OSD2Hurd
    {
        uint8_t h_i_frag;
        uint8_t h_i_fsize;
        uint16_t h_i_mode_high;
        uint16_t h_i_uid_high;
        uint16_t h_i_gid_high;
        uint32_t h_i_author;
    };

    struct OSD2Linux
    {
        uint8_t h_i_frag;
        uint8_t h_i_fsize;
        uint16_t h_i_reserved1;
        uint16_t h_i_uid_high;
        uint16_t h_i_gid_high;
        uint32_t h_i_reserved2;
    };

    struct OSD2Masix
    {
        uint8_t h_i_frag;
        uint8_t h_i_fsize;
        uint8_t h_i_reserved[10];
    };

    struct INode
    {
        uint16_t i_mode;
        uint16_t i_uid;
        uint32_t i_size;
        uint32_t i_atime;
        uint32_t i_ctime;
        uint32_t i_mtime;
        uint32_t i_dtime;
        uint16_t i_gid;
        uint16_t i_links_count;
        uint32_t i_blocks;
        uint32_t i_flags;
        uint32_t i_osd1;
        uint32_t i_block[15];
        uint32_t i_generation;
        uint32_t i_file_acl;
        uint32_t i_dir_acl;
        uint32_t i_faddr;
        union
        {
            uint8_t i_osd2[12];
            OSD2Hurd i_osd2_hurd;
            OSD2Linux i_osd2_linux;
            OSD2Masix i_osd2_masix;
        };
    };

    struct DirectoryEntry
    {
        uint32_t inode;
        uint16_t rec_len;
        uint8_t name_len;
        union
        {
            uint8_t file_type;
            uint8_t name_len_high;
        };
        char name[0];
    };
#pragma pack(pop)
private:
    class FSINode : public ::INode
    {
        friend class EXT2;
        EXT2::INode Data;
        FSINode(ino_t number, FileSystem *fs);
        void setSize(uint64_t size);
        bool buildDirectory(FSINode *parentINode, FSINode *newINode);

        virtual uint64_t GetSize();
        virtual mode_t GetMode();
        virtual time_t GetCreateTime();
        virtual time_t GetModifyTime();
        virtual time_t GetAccessTime();
        virtual int GetLinkCount();
        virtual uid_t GetUID();
        virtual gid_t GetGID();
        virtual bool SetCreateTime(time_t t);
        virtual bool SetModifyTime(time_t t);
        virtual bool SetAccessTime(time_t t);
        virtual bool Create(const char *name, mode_t mode);
        virtual int64_t Read(void *buffer, int64_t position, int64_t n);
        virtual int64_t Write(const void *buffer, int64_t position, int64_t n);
        virtual ::DirectoryEntry *ReadDir(int64_t position, int64_t *newPosition);
        virtual int64_t Resize(int64_t size);
        virtual int Remove(const char *name);
        virtual int Release();
    };

    SuperBlock *superBlock;
    bool readOnly;
    size_t blockSize;
    size_t fragSize;
    uint64_t totalSize;
    size_t blockGroupCount;
    BlockGroupDescriptor *BGDT;
    size_t BGDTSize;
    int64_t BGDTOffset;
    bool initialized;
    bool superDirty;
    uint8_t *blockOfZeros;

    static uint8_t modeToFileType(uint32_t mode);
    static bool isValidFileName(const char *name);

    EXT2(class Volume *vol, FileSystemType *type, SuperBlock *sblock, bool ro);
    uint64_t getINodeOffset(ino_t n);
    uint64_t blockGroupOffset(uint bg);
    bool hasSuperBlock(uint bg);
    uint64_t bgdtOffset(uint bg);
    bool updateBGDT(uint bg, off_t startOffs, size_t n);
    uint getINodeBlockGroup(uint32_t ino);
    uint32_t allocINode(uint *group);
    bool freeINode(uint32_t inode);
    uint32_t allocBlockInGroup(uint g);
    uint32_t allocBlock(uint preferredGroup, uint *group);
    bool freeBlock(uint32_t block);
    int64_t read(FSINode *inode, void *buffer, uint64_t position, int64_t n);
    int64_t write(FSINode *inode, const void *buffer, uint64_t position, int64_t n);
    uint32_t getINodeBlock(FSINode *inode, uint32_t n);
    bool isBlockZeroed(uint32_t block);
    bool setINodeBlock(FSINode *inode, uint32_t n, uint32_t block);
    bool zeroBlock(uint32_t block);
    virtual ~EXT2();
public:
    virtual bool GetLabel(char *buffer, size_t bufSize);
    virtual UUID GetUUID();
    virtual ::INode *ReadINode(ino_t number);
    virtual bool WriteINode(::INode *inode);
    virtual bool WriteSuperBlock();
};
