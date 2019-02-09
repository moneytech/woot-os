#ifndef SYSDEFS_H
#define SYSDEFS_H

#define KERNEL_VERSION_MAJOR        0
#define KERNEL_VERSION_MINOR        3
#define KERNEL_VERSION_DESCRIPTION  "internal"

#define KERNEL_BASE                 0xC0000000
#define MODULES_BASE                0xC8000000
#define MMIO_BASE                   0xD0000000

#define PAGE_SIZE                   0x00001000
#define LARGE_PAGE_SIZE             0x00400000
#define SMALL_PAGES_PER_LARGE_PAGE  (LARGE_PAGE_SIZE / PAGE_SIZE)
#define KERNEL_SPACE_SIZE           0x10000000
#define MODULES_SPACE_SIZE          (MMIO_BASE - MODULES_BASE - LARGE_PAGE_SIZE)
#define DMA_HEAP_START              0xCE000000
#define PDE_SIZE                    4
#define PTE_SIZE                    4
#define PDE_PER_TABLE               1024
#define PTE_PER_TABLE               1024
#define DEFAULT_STACK_SIZE          (64 << 10)
#define DEFAULT_USER_STACK_SIZE     (256 << 10)
#define DEFAULT_SIGNAL_STACK_SIZE   (16 << 10)

// filesystems
#define PATH_SEPARATORS "/\\"
#define VOLUME_SEPARATOR ':'
#define MAX_FS_LABEL_LENGTH 127
#define MAX_PATH_LENGTH 4095

// syscalls
#define SYSCALLS_INT_VECTOR 128

// segments
#define SEG_CODE32_KERNEL   0x0008
#define SEG_DATA32_KERNEL   0x0010
#define SEG_CODE32_USER     0x001B
#define SEG_DATA32_USER     0x0023
#define SEG_TSS             0x002B

#define GDT_ENTRY_COUNT ((SEG_TSS + 8) / 8)

#define USERCODE __attribute__((section(".utext")))
#define USERDATA __attribute__((section(".udata")))
#define USERBSS __attribute__((section(".ubss")))

#endif // SYSDEFS_H
