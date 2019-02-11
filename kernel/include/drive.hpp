#pragma once

#include <objecttree.hpp>
#include <sequencer.hpp>
#include <types.hpp>

#define DRV_DIR "/dev/drv"

class Drive : public ObjectTree::Item
{
    static Sequencer<int> id;
protected:
    Drive(size_t sectorSize, uint64_t sectorCount, const char *model, const char *serial);
public:
    int ID;
    size_t SectorSize;
    uint64_t SectorCount;
    uint64_t Size;
    char *Model;
    char *Serial;

    virtual int64_t ReadSectors(void *buffer, uint64_t start, int64_t count);
    virtual int64_t WriteSectors(const void *buffer, uint64_t start, int64_t count);
    virtual bool HasMedia();
    virtual bool KeyCheck(const char *name);
    virtual void GetDisplayName(char *buf, size_t bufSize);
    virtual ~Drive();
};
