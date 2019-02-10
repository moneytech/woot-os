#pragma once

class Character
{
public:
    static int ToLower(int c);
    static int ToUpper(int c);
    static bool IsDigit(int c);
    static bool IsSpace(int c);
    static bool IsHexDigit(int c);
    static bool IsAlpha(int c);
    static bool IsUpper(int c);
    static bool IsLower(int c);
    static bool IsAlphanumeric(int c);
    static bool IsPrintable(int c);
    static bool IsGraphical(int c);
    static bool IsControl(int c);
};
