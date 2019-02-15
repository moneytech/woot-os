#include <character.hpp>
#include <misc.hpp>
#include <stream.hpp>
#include <string.hpp>
#include <uuid.hpp>

const static char *decTable = "0123456789";

char Stream::ReadChar()
{
    char c;
    int64_t br = Read(&c, 1);
    if(br <= 0) return 0;
    return c;
}

int64_t Stream::ReadLine(char *buffer, int64_t maxLen)
{
    if(!maxLen)
        return 0;
    int64_t i;
    for(i = 0; i < maxLen - 1;)
    {
        char c = ReadChar();
        if(c == 127)
        { // backspace
            if(i) i--;
            continue;
        }
        else if(!c)
        {
            if(buffer) buffer[i] = c;
            return i;
        }
        else if(c == '\n' || c == '\r')
            break;
        if(buffer) buffer[i] = c;
        ++i;
    }
    if(buffer) buffer[i] = 0;
    ++i;
    return i;
}

int64_t Stream::WriteStr(const char *str, int64_t maxLen)
{
    int64_t i = String::Length(str);
    if(maxLen && i > maxLen) i = maxLen;
    return Write(str, i);
}

int64_t Stream::WriteHex(uint64_t value, bool caps, uint minDigits, uint maxDigits, bool measure)
{
    const static char *hexTableLo = "0123456789abcdef";
    const static char *hexTableUp = "0123456789ABCDEF";

    if(minDigits < 1)
        minDigits = 1;
    else if(minDigits > 16)
        minDigits = 16;
    if(maxDigits < 1)
        maxDigits = 1;
    else if(minDigits > 16)
        maxDigits = 16;
    if(minDigits > maxDigits)
        maxDigits = minDigits;

    char buf[20] = { 'X' };
    bool nonzero = false;
    int i, j;
    for(i = 0, j = 0; i < 16; i++, value <<= 4)
    {
        int dig = (value >> 60) & 0x0F;
        nonzero |= dig != 0;
        char c = caps ? hexTableUp[dig] : hexTableLo[dig];
        if((16 - i <= maxDigits && nonzero) || 16 - i <= minDigits)
            buf[j++] = c;
    }
    buf[j] = 0;
    return measure ? String::Length(buf) : WriteStr(buf, 0);

}

int64_t Stream::WriteDec(uint64_t value, uint minDigits, uint maxDigits, bool showPlus, bool measure, bool unsig)
{
    if(minDigits < 1)
        minDigits = 1;
    else if(minDigits > 19)
        minDigits = 19;
    if(maxDigits < 1)
        maxDigits = 19;
    else if(maxDigits > 19)
        maxDigits = 19;
    if(minDigits > maxDigits)
        maxDigits = minDigits;

    char buf[24] = { 'X' };
    bool nonzero = false;
    int i, j = 0;
    int64_t d = 1000000000000000000;
    if(showPlus && value >= 0)
        buf[j++] = '+';
    else if(!unsig && ((int64_t)value) < 0)
    {
        value = -value;
        buf[j++] = '-';
    }
    for(i = 0; i < 19 && d != 0; i++, d /= 10)
    {
        int dig = value / d;
        value -= dig * d;
        nonzero |= dig != 0;
        char c = decTable[dig % 10];
        if((19 - i <= maxDigits && nonzero) || 19 - i <= minDigits)
            buf[j++] = c;
    }
    buf[j] = 0;
    return measure ? String::Length(buf) : WriteStr(buf, 0);
}

int64_t Stream::WriteFmt(const char *fmt, ...)
{
    int64_t r = 0;
    VarArgs args;
    VarArgStart(args, fmt);
    r = VWriteFmt(fmt, args);
    VarArgEnd(args);
    return r;
}

int64_t Stream::VWriteFmt(const char *fmt, VarArgs args)
{
    bool hashFlag = false;
    bool dot = false;
    int width = 0;
    int precision = 0;
    bool specifier = false;
    bool argWidth = false;
    bool leftJustify = false;
    bool zeroPad = false;
    char padChr = ' ';
    bool showPlus = false;
    bool longSpec = false;
    int64_t bw = 0;
    while(*fmt)
    {
        char c = *fmt;
        if(!specifier)
        {
            width = 0;
            hashFlag = false;
            dot = false;
            precision = 0;
            argWidth = false;
            leftJustify = false;
            zeroPad = false;
            padChr = ' ';
            showPlus = false;
            longSpec = false;
            if(c == '%')
                specifier = true;
            else
                bw += Write(&c, 1);
        }
        else
        {
            if(c == '%')
            {
                bw += Write(&c, 1);
                specifier = false;
            }
            else if(c == 'l')
                longSpec = true;
            else if(c == '.')
            {
                if(!dot)
                    dot = true;
                else
                {
                    bw += Write(&c, 1);
                    specifier = false;
                }
            }
            else if(c == '#')
            {
                if(!hashFlag && !dot)
                    hashFlag = true;
                else
                {
                    bw += Write(&c, 1);
                    specifier = false;
                }
            }
            else if(c >= '0' && c <= '9')
            {
                if(!dot)
                {
                    if(!width && c == '0' && !zeroPad)
                    {
                        padChr = '0';
                        zeroPad = true;
                    }
                    else
                    {
                        width *= 10;
                        width += c - '0';
                    }
                }
                else
                {
                    precision *= 10;
                    precision += c - '0';
                }
            }
            else if(c == '*')
            {
                if(!argWidth && !width)
                {
                    argWidth = true;
                    width = VarArg(args, int);
                }
                else
                {
                    bw += Write(&c, 1);
                    specifier = false;
                }

            }
            else if(c == '-')
            {
                if(!width && !dot && !leftJustify)
                    leftJustify = true;
                else
                {
                    bw += Write(&c, 1);
                    specifier = false;
                }
            }
            else if(c == '+')
            {
                if(!width && !dot && !showPlus)
                    showPlus = true;
                else
                {
                    bw += Write(&c, 1);
                    specifier = false;
                }
            }
            else if(c == 's' || c == 'S')
            {
                const char *str = VarArg(args, char *);
                if(!str) str = (c == 'S' ? (const char *)L"(null)" : "(null)");
                const wchar_t *STR = (wchar_t *)str;
                int len = c == 's' ? String::Length(str) : String::Length(STR);
                if(dot) width = precision;
                int padc = width - len;
                padc = (padc < 0 || dot) ? 0 : padc;

                if(!leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                if(c == 's')
                    bw += WriteStr(str, width);
                else if(c == 'S')
                {
                    for(int i = 0; (!width || i < width) && *STR; i++)
                    {
                        bw += Write(STR, 1);
                        STR++;
                    }
                }

                if(leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                specifier = false;
            }
            else if(c == 'c')
            {
                char chr = VarArg(args, int);
                int padc = width - 1;
                padc = padc < 0 ? 0 : padc;

                if(!leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                bw += Write(&chr, 1);

                if(leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                specifier = false;
            }
            else if(c == 'p' || c == 'P' || c == 'x' || c == 'X')
            {
                if(c == 'p' || c == 'P')
                {
                    dot = true;
                    hashFlag = true;
                    precision = sizeof(void *) * 2;
                }

                bool upperCase = Character::IsUpper(c);
                uint64_t val = longSpec ? VarArg(args, uint64_t) : VarArg(args, uint);
                int maxDigits = width ? width - (hashFlag ? 2 : 0) : 16;
                int len = WriteHex(val, upperCase, precision, maxDigits, true);
                len += hashFlag ? 2 : 0;
                int padc = width - len;
                padc = padc < 0 ? 0 : padc;

                if(!leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                if(hashFlag)
                    bw += WriteStr("0x", 0);
                bw += WriteHex(val, upperCase, precision, maxDigits, false);

                if(leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                specifier = false;
            }
            else if(c == 'd' || c == 'i' || c == 'u')
            {
                int64_t val;

                if(c == 'u')
                    val = longSpec ? VarArg(args, uint64_t) :  VarArg(args, uint32_t);
                else
                    val = longSpec ? VarArg(args, int64_t) :  VarArg(args, int32_t);

                int maxDigits = width ? width : 19;
                int len = WriteDec(val, precision, maxDigits, showPlus, true, c == 'u');
                int padc = width - len;
                padc = padc < 0 ? 0 : padc;

                if(!leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                bw += WriteDec(val, precision, maxDigits, showPlus, false, c == 'u');

                if(leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                specifier = false;
            }
            else if(c == 'f')
            {
                double val = VarArg(args, double);
                if(val != val)
                    bw += WriteStr("NaN", 0);
                else if(val / val != val / val)
                    bw += WriteStr(val < 0 ? "-Inf" : "+Inf", 0);
                else
                {
                    int64_t i = (int64_t)(val);
                    bw += WriteDec(i, 1, -1, showPlus, false, false);
                    bw += WriteByte('.');
                    double f = val - i;
                    if(precision) precision = precision > 19 ? 19 : precision;
                    else precision = 4;
                    int64_t ai = (int64_t)(f * Misc::PowULL(10, precision));
                    ai = ai < 0 ? - ai : ai;
                    bw += WriteDec(ai, precision, -1, false, false, false);
                }
                specifier = false;
            }
            else if(c == 'U') // UUID
            {
                UUID *uuid = VarArg(args, UUID *);
                if(!uuid) uuid = &UUID::nil;
                char uuidStr[40];
                uuid->ToString(uuidStr);
                if(hashFlag)
                    bw += WriteByte('{');
                bw += WriteStr(uuidStr);
                if(hashFlag)
                    bw += WriteByte('}');
                specifier = false;
            }
            else
            {
                bw += Write(&c, 1);
                specifier = false;
            }
        }
        fmt++;
    }
    return bw;
}

int64_t Stream::WriteByte(uint8_t value)
{
    return Write(&value, 1);
}

uint8_t Stream::ReadByte(int64_t *result)
{
    uint8_t b;
    int64_t br = Read(&b, 1);
    if(result) *result = br;
    return b;
}

Stream::~Stream()
{
}
