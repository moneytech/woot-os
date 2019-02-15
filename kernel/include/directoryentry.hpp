#pragma once

#include <ktypes.h>
#include <types.hpp>

// general class for directory listing
class DirectoryEntry
{
public:
    mode_t Mode;
    time_t AccessTime;
    time_t CreateTime;
    time_t ModifyTime;
    uint64_t Size;
    ino_t INode;
    char *Name;

    DirectoryEntry(mode_t mode, time_t atime, time_t ctime, time_t mtime, uint64_t size, ino_t inode, const char *name);
    ~DirectoryEntry();
};
