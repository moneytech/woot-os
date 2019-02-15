#include <character.hpp>
#include <errno.h>
#include <memory.hpp>
#include <misc.hpp>
#include <string.hpp>

size_t String::Length(const char *str)
{
    if(!str) return 0;
    const char *s = str;
    for (; *s; ++s);
    return s - str;
}

size_t String::Length(const wchar_t *str)
{
    if(!str) return 0;
    const wchar_t *s = str;
    for (; *s; ++s);
    return s - str;
}

int String::Compare(const char *s1, const char *s2)
{
    for(;;)
    {
        unsigned char a = *s1++;
        unsigned char b = *s2++;
        int d = a - b;
        if(d) return d;
        else if(!a) break;
    };
    return 0;
}

int String::Compare(const char *s1, const char *s2, size_t n)
{
    for(size_t i = 0; i < n; ++i)
    {
        unsigned char a = *s1++;
        unsigned char b = *s2++;
        int d = a - b;
        if(d) return d;
        else if(!a) break;
    };
    return 0;
}

char *String::Duplicate(const char *s)
{
    if(!s) return nullptr;
    int len = Length(s) + 1;
    char *str = new char[len];
    Memory::Move(str, s, len);
    return str;
}

char *String::Copy(char *dst, const char *src)
{
    char *ret = dst;
    while((*dst++ = *src++));
    return ret;
}

char *String::Copy(char *dst, const char *src, size_t n)
{
    Memory::Zero(dst, n);
    char *ret = dst;
    for(size_t i = 0; i < n && (*dst++ = *src++); ++i);
    return ret;
}

size_t String::Span(const char *s1, const char *s2)
{
    size_t ret = 0;
    while(*s1 && Find(s2, *s1++, false))
        ret++;
    return ret;
}

size_t String::CharacterSpan(const char *s1, const char *s2)
{
    size_t ret = 0;
    while(*s1)
    {
        if(Find(s2, *s1, false))
            return ret;
        else s1++, ret++;
    }
    return ret;
}

char *String::Concatenate(char *dst, const char *src)
{
    char *ret = dst;
    while(*dst) dst++;
    while((*dst++ = *src++));
    return ret;
}

char *String::Concatenate(char *dst, const char *src, size_t n)
{
    char *ret = dst;
    size_t i = 0;
    for(;i < n && (*dst); dst++);
    for(;i < n && ((*dst++ = *src++)););
    return ret;
}

char *String::Tokenize(char *str, const char *delim, char **nextp)
{
    char *ret;
    if(!str) str = *nextp;
    str += Span(str, delim);
    if(!*str) return nullptr;
    ret = str;
    str += CharacterSpan(str, delim);
    if(*str) *str++ = 0;
    *nextp = str;
    return ret;
}

char *String::Find(const char *s, int c, bool reverse)
{
    if(reverse)
    {
        char *ret = 0;
        do
        {
            if(*s == (char)c)
                ret = (char *)s;
        } while(*s++);
        return ret;
    }
    while(*s != (char)c)
    {
        if(!*s++)
            return 0;
    }
    return (char *)s;
}

char *String::Find(const char *haystack, const char *needle)
{
    const char *a, *b = needle;
    if(!*b) return (char *)haystack;

    for(; *haystack; ++haystack)
    {
        if(*haystack != *b)
            continue;
        a = haystack;
        for(;;)
        {
            if(!*b) return (char *)haystack;
            if(*a++ != *b++)
                break;
        }
        b = needle;
    }
    return nullptr;
}

long String::ToLong(const char *str, char **endptr, int base)
{
    while(!Character::IsDigit(*str) && *str != '+' && *str != '-') str++; // skip leading non-digits
    long sign = *str == '-' ? ++str, -1 : (*str == '+' ? ++str, 1 : 1);
    if((!base || base == 16) && *str == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        str += 2;
        base = 16;
    }
    else if((!base || base == 2) && *str == '0' && (str[1] == 'b' || str[1] == 'B'))
    {
        str += 2;
        base = 2;
    }
    if(!base)
        base = *str == '0' ? 8 : 10; // select default base
    long result = 0;
    for(char c = Character::ToLower(*str); c; c = Character::ToLower(*(++str)))
    {
        int digit = 0;
        if(c >= '0' && c <= '9')
            digit = c - '0';
        else if(c >= 'a' && c <= 'z')
            digit = c - 'a';
        else break;
        if(digit >= base)
            break;
        long val = result * base + digit * sign;
        if(sign > 0)
        {
            if(val < result)
            {
                errno = ERANGE;
                result = __LONG_MAX__;
                break;
            }
        }
        else
        {
            if(val > result)
            {
                errno = ERANGE;
                result = -__LONG_MAX__ - 1;
                break;
            }
        }
        result = val;
    }
    if(endptr) *endptr = (char *)str;
    return result;
}

unsigned long String::ToULong(const char *str, char **endptr, int base)
{
    while(!Character::IsDigit(*str)) str++; // skip leading non-digits
    if((!base || base == 16) && *str == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        str += 2;
        base = 16;
    }
    else if((!base || base == 2) && *str == '0' && (str[1] == 'b' || str[1] == 'B'))
    {
        str += 2;
        base = 2;
    }
    if(!base)
        base = *str == '0' ? 8 : 10; // select default base
    unsigned long result = 0;
    for(char c = Character::ToLower(*str); c; c = Character::ToLower(*(++str)))
    {
        int digit = 0;
        if(c >= '0' && c <= '9')
            digit = c - '0';
        else if(c >= 'a' && c <= 'z')
            digit = c - 'a';
        else break;
        if(digit >= base)
            break;
        unsigned long val = result * base + digit;
        if(val < result)
        {
            errno = ERANGE;
            result = ~0ul;
            break;
        }
        result = val;
    }
    if(endptr) *endptr = (char *)str;
    return result;
}

long long String::ToLLong(const char *str, char **endptr, int base)
{
    while(!Character::IsDigit(*str) && *str != '+' && *str != '-') str++; // skip leading non-digits
    long long sign = *str == '-' ? ++str, -1 : (*str == '+' ? ++str, 1 : 1);
    if((!base || base == 16) && *str == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        str += 2;
        base = 16;
    }
    else if((!base || base == 2) && *str == '0' && (str[1] == 'b' || str[1] == 'B'))
    {
        str += 2;
        base = 2;
    }
    if(!base)
        base = *str == '0' ? 8 : 10; // select default base
    long long result = 0;
    for(char c = Character::ToLower(*str); c; c = Character::ToLower(*(++str)))
    {
        int digit = 0;
        if(c >= '0' && c <= '9')
            digit = c - '0';
        else if(c >= 'a' && c <= 'z')
            digit = c - 'a';
        else break;
        if(digit >= base)
            break;
        long long val = result * base + digit * sign;
        if(sign > 0)
        {
            if(val < result)
            {
                errno = ERANGE;
                result = __LONG_LONG_MAX__;
                break;
            }
        }
        else
        {
            if(val > result)
            {
                errno = ERANGE;
                result = -__LONG_LONG_MAX__ - 1;
                break;
            }
        }
        result = val;
    }
    if(endptr) *endptr = (char *)str;
    return result;
}

unsigned long long String::ToULLong(const char *str, char **endptr, int base)
{
    while(!Character::IsDigit(*str)) str++; // skip leading non-digits
    if((!base || base == 16) && *str == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        str += 2;
        base = 16;
    }
    else if((!base || base == 2) && *str == '0' && (str[1] == 'b' || str[1] == 'B'))
    {
        str += 2;
        base = 2;
    }
    if(!base)
        base = *str == '0' ? 8 : 10; // select default base
    unsigned long long result = 0;
    for(char c = Character::ToLower(*str); c; c = Character::ToLower(*(++str)))
    {
        int digit = 0;
        if(c >= '0' && c <= '9')
            digit = c - '0';
        else if(c >= 'a' && c <= 'z')
            digit = c - 'a';
        else break;
        if(digit >= base)
            break;
        unsigned long long val = result * base + digit;
        if(val < result)
        {
            errno = ERANGE;
            result = ~0ull;
            break;
        }
        result = val;
    }
    if(endptr) *endptr = (char *)str;
    return result;
}

double String::ToDouble(const char *str, char **endptr)
{
    while(Character::IsSpace(*str)) str++; // skip leading spaces
    double sign = *str == '-' ? ++str, -1.0 : (*str == '+' ? ++str, 1.0 : 1.0);
    double result = 0.0;
    double esign = 1.0;
    int f = 0;  // parsing fractional part
    int e = 0;  // parsing exponent part
    int es = 0; // encountered exponent sign
    int ev = 0; // exponent value
    for(char c = Character::ToLower(*str); c; c = Character::ToLower(*(++str)))
    {
        if(c == '.')
        {
            if(f) return 0.0;
            f = 1;
            continue;
        }
        else if(c == 'e')
        {
            if(e) return 0.0;
            e = 1;
            continue;
        }
        else if(c == '-' || c == '+')
        {
            if(!e || es) return 0.0;
            es = 1;
            esign = c == '-' ? -1.0 : 1.0;
            continue;
        }
        else if(c >= '0' || c <= '9')
        {
            int digit = c - '0';
            if(!f) result = result * 10 + digit;
            else if(f && !e) result = result + (double)digit / (double)Misc::PowULL(10, f++);
            else if(e) ev = ev * 10 + digit;
            else return 0.0;
        }
        else return 0.0;
    }
    result = result * sign * Misc::PowULL(10, ev * esign);
    return result;
}
