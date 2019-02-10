#pragma once

#include <types.hpp>

class String
{
public:
    static size_t Length(const char *str);
    static size_t Length(const wchar_t *str);
    static int Compare(const char *s1, const char *s2);
    static int Compare(const char *s1, const char *s2, size_t n);
    static char *Duplicate(const char *s);
    static char *Copy(char *dst, const char *src);
    static char *Copy(char *dst, const char *src, size_t n);
    static size_t Span(const char *s1, const char *s2);
    static size_t CharacterSpan(const char *s1, const char *s2);
    static char *Concatenate(char *dst, const char *src);
    static char *Concatenate(char *dst, const char *src, size_t n);
    static char *Tokenize(char *str, const char *delim, char **nextp);
    static char *Find(const char *s, int c, bool reverse);
    static char *Find(const char *haystack, const char *needle);
};
