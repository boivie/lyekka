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


const char* sha256_base16(const unsigned char* hash_p, char* base16_p)
{
  static const char translate[] = "0123456789abcdef";

  for (int i = 0; i < 256/8; i++)
  {
    base16_p[2 * i + 0] = translate[(hash_p[i] & 0xF0) >> 4];
    base16_p[2 * i + 1] = translate[(hash_p[i] & 0x0F)];
  }
  base16_p[256/4] = 0;
  return base16_p;
}
