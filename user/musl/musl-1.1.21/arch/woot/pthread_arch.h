#include <sys/syscall.h>

static inline struct pthread *__pthread_self()
{
    struct pthread *self;
    long res = syscall(SYS_GET_PTHREAD, &self);
    return res < 0 ? (struct pthread *)0 : self;
}

#define TP_ADJ(p) (p)

#define MC_PC gregs[REG_EIP]
