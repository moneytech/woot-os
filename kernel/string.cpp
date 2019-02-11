#include <memory.hpp>
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
