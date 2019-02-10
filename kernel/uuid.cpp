#include <character.hpp>
#include <memory.hpp>
#include <random.hpp>
#include <uuid.hpp>

UUID UUID::nil("00000000-0000-0000-0000-000000000000");

UUID::UUID()
{
    for(int i = 0; i < 16; ++i)
        Data[i] = Random::GetValue();

    // version 4 variant 1
    time_hi_and_version &= 0x0FFF;
    time_hi_and_version |= 0x4000;
    clock_seq_hi_and_res_clock_seq_low &= 0x3FFF;
    clock_seq_hi_and_res_clock_seq_low += 0x8000;
}

UUID::UUID(void *data)
{
    Memory::Move(Data, data, sizeof(Data));
}

UUID::UUID(const char *str)
{
    int digit = 0;
    while(const uint8_t c = *str++)
    {
        if(!c || digit >= 32)
            break;
        if(!Character::IsHexDigit(c))
            continue;
        int bt = digit >> 1;
        Data[bt] <<= 4;
        Data[bt] |= c >= '0' && c <= '9' ? c - '0' : (c >= 'a' && c <= 'f' ? c - 'a' : c - 'A') + 10;
        ++digit;
    }
}

UUID::UUID(const UUID &src)
{
    Memory::Move(Data, src.Data, sizeof(Data));
}

void UUID::ToString(char *buffer)
{
    static const char *hexTable = "0123456789abcdef";
    for(int i = 0, j = 0; i < 32; ++i, ++j)
    {
        if(i == 8 || i == 12 || i == 16 || i == 20)
            buffer[j++] = '-';
        buffer[j] = hexTable[(Data[i >> 1] >> (4 * (~i & 1))) & 0x0F];
    }
    buffer[36] = 0;
}

bool UUID::operator ==(UUID &uuid)
{
    return !Memory::Compare(Data, uuid.Data, sizeof(Data));
}

bool UUID::operator !=(UUID &uuid)
{
    return Memory::Compare(Data, uuid.Data, sizeof(Data));
}
