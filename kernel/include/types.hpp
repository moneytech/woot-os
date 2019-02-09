#pragma once

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#ifdef __i386__
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
#endif // __i386__

typedef intptr_t ssize_t;
typedef uintptr_t size_t;
