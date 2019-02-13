#pragma once

#define FS_TYPE_DIR "/sys/fstype"

#include <objecttree.hpp>
#include <types.hpp>

class Volume;

class FileSystemType : public ObjectTree::Item
{
protected:
    const char *name;

    FileSystemType(const char *name, bool autoRegister);
public:
    bool Register();
    bool UnRegister();

    virtual int Detect(Volume *volume);
    virtual void GetDisplayName(char *buf, size_t bufSize);
};
