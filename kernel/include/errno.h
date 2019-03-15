#pragma once

#include <errnovalues.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern int *_get_errno_ptr();

#ifdef __cplusplus
}
#endif // __cplusplus

#define errno (*(_get_errno_ptr()))
