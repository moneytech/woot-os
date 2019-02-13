#pragma once

#define VOLUME_DIR  "/dev/vol"

#include <objecttree.hpp>
#include <sequencer.hpp>
#include <types.hpp>

class Drive;
class FileSystem;
class VolumeType;

class Volume : public ObjectTree::Item
{
    static Sequencer<int> ids;
protected:
    int id;
    Drive *drive;
    FileSystem *fileSystem;
    VolumeType *type;

    Volume(Drive *drive, VolumeType *type);
public:
    static int DetectAll();

    bool HasFileSystem();

    virtual size_t GetSectorSize();
    virtual uint64_t GetSectorCount();
    virtual ssize_t ReadSectors(void *buffer, uint64_t firstSector, size_t n);
    virtual ssize_t WriteSectors(const void *buffer, uint64_t firstSector, size_t n);
    virtual ssize_t Read(void *buffer, uint64_t position, size_t n);
    virtual ssize_t Write(const void *buffer, uint64_t position, size_t n);
    virtual int Synchronize();

    virtual bool KeyCheck(const char *name);
    virtual void GetDisplayName(char *buf, size_t bufSize);
};
