#define __SYSCALL_LL_E(x) \
((union { long long ll; long l[2]; }){ .ll = x }).l[0], \
((union { long long ll; long l[2]; }){ .ll = x }).l[1]
#define __SYSCALL_LL_O(x) __SYSCALL_LL_E((x))

static __attribute__((noinline)) long __syscall0(long n)
{
    __asm__ __volatile__("sysenter");
    return 0;
}

static __attribute__((noinline)) long __syscall1(long n, long a1)
{
    __asm__ __volatile__("sysenter");
    return 0;
}

static __attribute__((noinline)) long __syscall2(long n, long a1, long a2)
{
    __asm__ __volatile__("sysenter");
    return 0;
}

static __attribute__((noinline)) long __syscall3(long n, long a1, long a2, long a3)
{
    __asm__ __volatile__("sysenter");
    return 0;
}

static __attribute__((noinline)) long __syscall4(long n, long a1, long a2, long a3, long a4)
{
    __asm__ __volatile__("sysenter");
    return 0;
}

static __attribute__((noinline)) long __syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
    __asm__ __volatile__("sysenter");
    return 0;
}

static __attribute__((noinline)) long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
    __asm__ __volatile__("sysenter");
    return 0;
}

#define VDSO_USEFUL
#define VDSO_CGT_SYM "__vdso_clock_gettime"
#define VDSO_CGT_VER "WOOT_0.3"

#define SYSCALL_USE_SOCKETCALL
