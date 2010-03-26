#include <iostream>
#include <sys/mman.h>
#include <errno.h>

using namespace std;

void *xmmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	void *ret = mmap(start, length, prot, flags, fd, offset);
	if (ret == MAP_FAILED) {
		if (length == 0)
			return NULL;
    cerr << "mmap failed. " << strerror(errno) << endl;
    exit(124);
	}
	return ret;
}

