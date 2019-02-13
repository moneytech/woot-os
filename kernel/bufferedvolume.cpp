#include <bufferedvolume.hpp>
#include <debug.hpp>
#include <drive.hpp>
#include <errno.h>
#include <memory.hpp>
#include <paging.hpp>

Sequencer<uint64_t> BufferedVolume::lru(0);

int BufferedVolume::getContainingBuffer(uint64_t sector)
{
    for(int i = 0; i < bufferCount; ++i)
    {
        BlockBuffer *b = buffers + i;
        if(!b->Used)
            continue;
        if(sector >= b->FirstSector && sector < (b->FirstSector + sectorsPerBuffer))
            return i;
    }
    return -1;
}

bool BufferedVolume::flushBuffer(int bufIdx)
{
    if(bufIdx < 0 || bufIdx >= bufferCount)
        return false;
    BlockBuffer *b = buffers + bufIdx;
    if(!b->Used || !b->Dirty)
        return true;
    if(!b->Buffer) // buffer has some internal problem
        return false;
    if(WriteSectors(b->Buffer, b->FirstSector, sectorsPerBuffer) != sectorsPerBuffer)
        return false;
    b->Dirty = false;
    return true;
}

int BufferedVolume::allocateBuffer()
{
    BlockBuffer *b = nullptr;
    for(int i = 0; i < bufferCount; ++i)
    {
        b = buffers + i;
        if(!b->Used)
        {
            b->Used = true;
            return i;
        }
    }

    // no free buffer available at this point
    // one of them must be flushed and reused
    // LRU

    int minIdx = 0;
    uint64_t minTime = ~0ULL;
    for(int i = 0; i < bufferCount; ++i)
    {
        b = buffers + i;
        if(b->Time < minTime)
        {
            minTime = b->Time;
            minIdx = i;
        }
    }
    b = buffers + minIdx;
    if(b->Dirty)
    {
        bool flushed = flushBuffer(minIdx);
        if(!flushed) return -1;
    }
    return minIdx;
}

bool BufferedVolume::loadBuffer(int buffer, int64_t sector)
{
    if(!drive || !drive->SectorSize)
        return false;
    sector = sectorsPerBuffer * (sector / sectorsPerBuffer);
    BlockBuffer *b = buffers + buffer;
    if(!b->Buffer)
        b->Buffer = (uint8_t *)Paging::AllocDMA(sectorsPerBuffer * drive->SectorSize);
    b->Used = true;
    if(b->Dirty) // should be flushed already but just to be sure
    {
        bool flushed = flushBuffer(buffer);
        if(!flushed) return false;
    }
    int sr = ReadSectors(b->Buffer, sector, sectorsPerBuffer);
    if(sr < 0) return false;
    b->FirstSector = sector;
    return true;
}

int64_t BufferedVolume::access(uint8_t *buffer, int64_t n, int64_t position, bool write)
{
    if(!drive || !drive->SectorSize) return -EINVAL;
    uint64_t volSize = drive->SectorCount * drive->SectorSize;
    uint64_t blockSize = sectorsPerBuffer * drive->SectorSize;
    if((position + n) > volSize)
        n = volSize - position;
    if(n <= 0)
    { // nothing to transfer
        return 0;
    }

    int64_t bytesLeft = n;
    while(bytesLeft > 0)
    {
        int64_t inBlockOffset = position % blockSize;
        int64_t bytesToXfer = min(bytesLeft, blockSize - inBlockOffset);
        int64_t sector = position / drive->SectorSize;
        int bufIdx = getContainingBuffer(sector);
        if(bufIdx < 0)
        {
            bufIdx = allocateBuffer();
            if(bufIdx < 0)
            {
                char name[64]; drive->GetDisplayName(name, sizeof(name));
                DEBUG("[bufferedvolume] allocateBuffer failed for drive '%s'\n", name);
                return -ENOMEM;
            }
            if(!loadBuffer(bufIdx, sector))
            {
                char name[64]; drive->GetDisplayName(name, sizeof(name));
                DEBUG("[bufferesvolume] loadBuffer failed for drive '%s'\n", name);
                return -EIO;
            }
        }
        BlockBuffer *b = buffers + bufIdx;
        b->Time = lru.GetNext();
        if(buffer)
        {
            if(write)
            {
                Memory::Move(b->Buffer + inBlockOffset, buffer, bytesToXfer);
                b->Dirty = true;
            }
            else Memory::Move(buffer, b->Buffer + inBlockOffset, bytesToXfer);
            buffer += bytesToXfer;
        }
        bytesLeft -= bytesToXfer;
        position += bytesToXfer;
    }

    return n;
}

bool BufferedVolume::flushBuffers()
{
    bool res = true;
    for(int i = 0; i < bufferCount; ++i)
        res &= flushBuffer(i);
    return res;
}

BufferedVolume::BufferedVolume(class Drive *drive, VolumeType *type, size_t sectorsPerBuffer, size_t bufferCount) :
    Volume(drive, type),
    sectorsPerBuffer(sectorsPerBuffer),
    bufferCount(bufferCount),
    buffers(new BlockBuffer[bufferCount])
{
}

BufferedVolume::~BufferedVolume()
{
    flushBuffers();
    for(int i = 0; i < bufferCount; ++i)
    {
        if(buffers[i].Buffer)
            Paging::FreeDMA(buffers[i].Buffer);
    }
    delete[] buffers;
}

ssize_t BufferedVolume::Read(void *buffer, uint64_t position, size_t n)
{
    return access((uint8_t *)buffer, n, position, false);
}

ssize_t BufferedVolume::Write(const void *buffer, uint64_t position, size_t n)
{
    return access((uint8_t *)buffer, n, position, true);
}

int BufferedVolume::Synchronize()
{
    return flushBuffers();
}
