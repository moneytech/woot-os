#pragma once

#include <sequencer.hpp>
#include <types.hpp>
#include <volume.hpp>

class BufferedVolume : public Volume
{
    struct BlockBuffer
    {
        uint8_t *Buffer = nullptr;
        uint64_t FirstSector = 0;
        bool Used = false;
        bool Dirty = false;
        uint64_t Time = 0; // Used for LRU algorithm
    };

    static Sequencer<uint64_t> lru;

    size_t sectorsPerBuffer;
    size_t bufferCount;
    BlockBuffer *buffers;

    int getContainingBuffer(uint64_t sector);
    bool flushBuffer(int bufIdx);
    int allocateBuffer();
    bool loadBuffer(int buffer, int64_t sector);
    int64_t access(uint8_t *buffer, int64_t n, int64_t position, bool write);
    bool flushBuffers();
protected:
    BufferedVolume(class Drive *drive, VolumeType *type, size_t sectorsPerBuffer, size_t bufferCount);
    virtual ~BufferedVolume();
public:
    virtual ssize_t Read(void *buffer, uint64_t position, size_t n);
    virtual ssize_t Write(const void *buffer, uint64_t position, size_t n);
    virtual int Synchronize();
};
