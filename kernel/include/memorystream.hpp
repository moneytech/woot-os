#pragma once

#include <types.hpp>
#include <stream.hpp>

class MemoryStream : public Stream
{
protected:
    uint8_t *data = nullptr;
    size_t size = 0;
    size_t position = 0;
public:
    MemoryStream(void *base, size_t size);
    void *GetData();
    virtual int64_t Read(void *buffer, int64_t n);
    virtual int64_t Write(const void *buffer, int64_t n);
};
