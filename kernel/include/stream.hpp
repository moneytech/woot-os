#pragma once

#include <vararg.hpp>
#include <types.hpp>

class Stream
{
public:
    virtual int64_t Read(void *buffer, int64_t n) = 0;
    virtual int64_t Write(const void *buffer, int64_t n) = 0;
    char ReadChar();
    int64_t ReadLine(char *buffer, int64_t maxLen);
    int64_t WriteStr(const char *str, int64_t maxLen = 0);
    int64_t WriteHex(uint64_t value, bool caps, uint minDigits, uint maxDigits, bool measure);
    int64_t WriteDec(uint64_t value, uint minDigits, uint maxDigits, bool showPlus, bool measure, bool unsig);
    int64_t WriteFmt(const char *fmt, ...);
    int64_t VWriteFmt(const char *fmt, VarArgs args);
    int64_t WriteByte(uint8_t value);
    uint8_t ReadByte(int64_t *result);
    virtual ~Stream();
};
