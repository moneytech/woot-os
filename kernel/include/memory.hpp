#pragma once

#include <types.hpp>

void ZeroMemory(void *dst, size_t n);

void SetMemory(void *dst, int val, size_t n);
void SetMemory16(void *dst, int val, size_t n);
void SetMemory32(void *dst, int val, size_t n);
void SetMemory64(void *dst, int val, size_t n);

void MoveMemory(void *dst, const void *src, size_t n);
void MoveMemory2D(void *dst, const void *src, size_t bpl, size_t dstride, size_t sstride, size_t lines);

int CompareMemory(const void *ptr1, const void *ptr2, size_t n);
