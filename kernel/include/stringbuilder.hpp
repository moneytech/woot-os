#pragma once

#include <types.hpp>
#include <memorystream.hpp>

class StringBuilder : public MemoryStream
{
    bool deleteBuffer = false;
public:
    StringBuilder(size_t maxLength);
    StringBuilder(char *buffer, size_t bufferSize);
    bool Clear();
    char *String();
    size_t Length();
    void ChangeCase(bool upper);
    size_t Replace(char chr, char replacement);
    virtual ~StringBuilder();
};
