#include <character.hpp>

int Character::ToLower(int c)
{
    if(c >= 'A' && c <= 'Z')
        return c + 32;
    return c;
}

int Character::ToUpper(int c)
{
    if(c >= 'a' && c <= 'z')
        return c - 32;
    return c;
}

bool Character::IsDigit(int c)
{
    return c >= '0' && c <= '9';
}

bool Character::IsSpace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

bool Character::IsHexDigit(int c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool Character::IsAlpha(int c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

bool Character::IsUpper(int c)
{
    return (c >= 'A' && c <= 'Z');
}

bool Character::IsLower(int c)
{
    return (c >= 'a' && c <= 'z');
}

bool Character::IsAlphanumeric(int c)
{
    return IsAlpha(c) || IsDigit(c);
}

bool Character::IsPrintable(int c)
{
    return c >= ' ' && c != 0x7F;
}

bool Character::IsGraphical(int c)
{
    return c > ' ' && c != 0x7F;
}

bool Character::IsControl(int c)
{
    return !IsPrintable(c);
}
