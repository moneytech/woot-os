#pragma once

#define VOL_TYPE_DIR "/sys/voltype"

#include <objecttree.hpp>
#include <types.hpp>

class Drive;

class VolumeType : public ObjectTree::Item
{
protected:
    const char *name;

    VolumeType(const char *name, bool autoRegister);
public:
    virtual int Detect(Drive *drive);
    virtual void GetDisplayName(char *buf, size_t bufSize);
};
