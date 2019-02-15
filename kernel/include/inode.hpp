#pragma once

#include <ktypes.h>
#include <list.hpp>
#include <types.hpp>

class DEntry;
class DirectoryEntry;
class FileSystem;

class INode
{
public:
    ino_t Number;
    FileSystem *FS;
    int ReferenceCount;
    bool Dirty;

    INode(ino_t number, FileSystem *fs);
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
    virtual ino_t Lookup(const char *name);
    virtual int64_t Read(void *buffer, int64_t position, int64_t n);
    virtual int64_t Write(const void *buffer, int64_t position, int64_t n);
    virtual DirectoryEntry *ReadDir(int64_t position, int64_t *newPosition);
    virtual int64_t Resize(int64_t size);
    virtual int Remove(const char *name);
    virtual int Release();
    virtual ~INode();
};
