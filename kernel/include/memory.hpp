#pragma once

#include <types.hpp>

class Memory
{
public:
    static void Zero(void *dst, size_t n);
    static void Set(void *dst, int val, size_t n);
    static void Set16(void *dst, int val, size_t n);
    static void Set32(void *dst, int val, size_t n);
    static void Set64(void *dst, int val, size_t n);
    static void Move(void *dst, const void *src, size_t n);
    static void Move2D(void *dst, const void *src, size_t bpl, size_t dstride, size_t sstride, size_t lines);
    static int Compare(const void *ptr1, const void *ptr2, size_t n);
};
