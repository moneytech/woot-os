#include <character.hpp>
#include <memory.hpp>
#include <stringbuilder.hpp>

StringBuilder::StringBuilder(size_t maxLength) :
    MemoryStream(new char[maxLength + 1], maxLength + 1),
    deleteBuffer(true)
{
    Memory::Set(data, 0, maxLength + 1);
}

StringBuilder::StringBuilder(char *buffer, size_t bufferSize) :
    MemoryStream(buffer, bufferSize),
    deleteBuffer(false)
{
    Memory::Set(buffer, 0, bufferSize);
}

bool StringBuilder::Clear()
{
    position = 0;
    Memory::Set(data, 0, size);
    return true;
}

char *StringBuilder::String()
{
    return (char *)GetData();
}

size_t StringBuilder::Length()
{
    return position;
}

void StringBuilder::ChangeCase(bool upper)
{
    char *ptr = (char *)GetData();
    for(int i = 0; i < size && *ptr; ++i)
        *ptr++ = upper ? Character::ToUpper(*ptr) : Character::ToLower(*ptr);
}

size_t StringBuilder::Replace(char chr, char replacement)
{
    size_t res = 0;
    char *ptr = (char *)GetData();
    for(int i = 0; i < size && *ptr; ++i, ++ptr)
    {
        if(*ptr == chr)
        {
            *ptr = replacement;
            ++res;
        }
    }
    return res;
}

StringBuilder::~StringBuilder()
{
    if(deleteBuffer)
        delete[] (char *)data;
}
