#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "syscall.h"

char *getcwd(char *buf, size_t size)
{
	char tmp[buf ? 1 : PATH_MAX];
	if (!buf) {
		buf = tmp;
		size = sizeof tmp;
	} else if (!size) {
		errno = EINVAL;
		return 0;
	}
	long ret = syscall(SYS_getcwd, buf, size);
	if (ret < 0)
		return 0;
#ifdef __WOOT__
    // in WOOT absolute paths don't have to begin with '/'
    if (ret == 0) {
        errno = ENOENT;
        return 0;
    }
#else // __WOOT__
	if (ret == 0 || buf[0] != '/') {
		errno = ENOENT;
		return 0;
	}
#endif // __WOOT__
    return buf == tmp ? strdup(buf) : buf;
}
