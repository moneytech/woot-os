#include <stdio.h>
#include <syscall.h>

int a;

void syscall(int n, ...)
{
    asm("sysenter");
}

int main(int argc, char *argv[])
{
    printf("Hello from userland!\n");
    return 1;
}
