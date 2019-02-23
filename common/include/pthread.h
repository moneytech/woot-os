#pragma once

typedef struct __locale_struct *locale_t;

struct pthread
{
	/* Part 1 -- these fields may be external or
	 * internal (accessed via asm) ABI. Do not change. */
	struct pthread *self;
	uintptr_t *dtv;
	void *unused1, *unused2;
	uintptr_t sysinfo;
	uintptr_t canary, canary2;

	/* Part 2 -- implementation details, non-ABI. */
	int tid;
	int errno_val;
	volatile int detach_state;
	volatile int cancel;
	volatile unsigned char canceldisable, cancelasync;
	unsigned char tsd_used:1;
	unsigned char unblock_cancel:1;
	unsigned char dlerror_flag:1;
	unsigned char *map_base;
	size_t map_size;
	void *stack;
	size_t stack_size;
	size_t guard_size;
	void *start_arg;
	void *(*start)(void *);
	void *result;
	struct __ptcb *cancelbuf;
	void **tsd;
	struct {
		volatile void *volatile head;
		long off;
		volatile void *volatile pending;
	} robust_list;
	volatile int timer_id;
	locale_t locale;
	volatile int killlock[1];
	char *dlerror_buf;
	void *stdio_locks;

	/* Part 3 -- the positions of these fields relative to
	 * the end of the structure is external and internal ABI. */
	uintptr_t canary_at_end;
	uintptr_t *dtv_copy;
};

