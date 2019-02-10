#pragma once

#include <types.hpp>

void *operator new(size_t size, void *place);
void *operator new[](size_t size, void *place);
void *operator new(size_t size, size_t alignment);
void *operator new[](size_t size, size_t alignment);
