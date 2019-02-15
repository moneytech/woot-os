#pragma once

#include <kdefs.h>
#include <ktypes.h>
#include <types.hpp>

class DEntry;
class DirectoryEntry;
class Mutex;

class File
{
    static File *open(::DEntry *parent, const char *name, int flags);

    File(::DEntry *dentry, int flags, mode_t mode);
    int64_t getSize();
public:
    ::DEntry *DEntry;
    int Flags;
    int64_t Position;
    mode_t Mode;

    static File *Open(::DEntry *parent, const char *name, int flags);
    static File *Open(const char *name, int flags);

    int64_t GetSize();
    bool SetAccessTime(time_t time);
    bool SetModifyTime(time_t time);
    bool Create(const char *name, mode_t mode);
    int Remove(const char *name);
    int64_t Seek(int64_t offs, int loc);
    int64_t Read(void *buffer, int64_t n);
    int64_t Write(const void *buffer, int64_t n);
    int64_t Rewind();
    DirectoryEntry *ReadDir();
    ~File(); // used as close
};
