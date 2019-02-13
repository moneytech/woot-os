#pragma once

#define FS_DIR  "/dev/fs"

#include <objecttree.hpp>
#include <sequencer.hpp>
#include <types.hpp>

class Volume;
class FileSystemType;

class FileSystem : public ObjectTree::Item
{
    static Sequencer<int> ids;
protected:
    int id;
    Volume *volume;
    FileSystemType *type;

    FileSystem(Volume *volume, FileSystemType *type);
public:
    static int DetectAll();

    virtual int Synchronize();

    virtual bool KeyCheck(const char *name);
    virtual void GetDisplayName(char *buf, size_t bufSize);
};
