#ifndef LYEKKA_H_INCLUSION_GUARD
#define LYEKKA_H_INCLUSION_GUARD

#include <cstring>

extern void *xmmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
#endif
