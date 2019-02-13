#pragma once

#include <bufferedvolume.hpp>
#include <partvolume.hpp>
#include <types.hpp>
#include <volumetype.hpp>

class PartVolumeType : public VolumeType
{
public:
    PartVolumeType(bool autoRegister);
    virtual int Detect(Drive *drive);
};

class PartVolume : public BufferedVolume
{
    uint64_t firstSector;
    uint64_t sectorCount;
public:
    PartVolume(Drive *drive, VolumeType *vt, uint64_t firstSector, uint64_t sectorCount);

    virtual uint64_t GetSectorCount();
    virtual ssize_t ReadSectors(void *buffer, uint64_t firstSector, size_t n);
    virtual ssize_t WriteSectors(const void *buffer, uint64_t firstSector, size_t n);
    virtual ssize_t Read(void *buffer, uint64_t position, size_t n);
    virtual ssize_t Write(const void *buffer, uint64_t position, size_t n);
};
