#include <memory.hpp>
#include <memorystream.hpp>

MemoryStream::MemoryStream(void *base, size_t size) :
    data((uint8_t *)base), size(size)
{
}

void *MemoryStream::GetData()
{
    return data;
}

int64_t MemoryStream::Read(void *buffer, int64_t n)
{
    if(position + n > size)
        n = size - position;
    if(n == 1) *(uint8_t *)buffer = data[position];
    else Memory::Move(buffer, data + position, n);
    position += n;
    return n;
}

int64_t MemoryStream::Write(const void *buffer, int64_t n)
{
    if(position + n > size)
        n = size - position;
    if(n == 1) data[position] = *(uint8_t *)buffer;
    else Memory::Move(data + position, buffer, n);
    position += n;
    return n;
}
