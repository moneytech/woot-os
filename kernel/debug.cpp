#include <debug.hpp>

void Debug::BufferDump(void *ptr, size_t n)
{
    uint8_t *buf = (uint8_t *)ptr;
    for(uint j = 0; j < n; j += 16)
    {
        DEBUG("%.8x : ", j);
        for(uint i = 0; i < 16; ++i)
            DEBUG("%.2x ", buf[i + j]);
        DEBUG("| ");
        for(uint i = 0; i < 16; ++i)
        {
            uint8_t b = buf[i + j];
            DEBUG("%c", b >= ' ' && b < '\x7f' ? b : '.');
        }
        DEBUG("\n");
    }
}
