#include <iostream>
#include <sys/mman.h>
#include <errno.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <fcntl.h>

#include "lyekka.h"

using namespace std;
using namespace google::protobuf::io;

void* Lyekka::xmmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
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

int Lyekka::xopen(const char* fn_p, int oflag, int mode) {
  int fd;
  if (mode == 0) {
    fd = open(fn_p, oflag);
  } else {
    fd = open(fn_p, oflag, mode);
  }

  if (fd == -1) {
    cerr << "Failed to open " << fn_p << " : " << strerror(errno) << endl;
    throw RuntimeError();
  }
  return fd;
}

bool Lyekka::write_to_stream(ZeroCopyOutputStream* os_p, const void* src_p, size_t size) 
{
  void* dest_p;
  int dest_size;
  
  if (!os_p->Next(&dest_p, &dest_size))
    return false;

  while (dest_size < size) {
    memcpy(dest_p, src_p, dest_size);
    size -= dest_size;
    src_p = (char*)(src_p) + dest_size;
    if (!os_p->Next(&dest_p, &dest_size))
      return false;
  }

  memcpy(dest_p, src_p, size);
  os_p->BackUp(dest_size - size);
  return true;
}

void Lyekka::copy_streams(google::protobuf::io::ZeroCopyOutputStream* os_p,
			  google::protobuf::io::ZeroCopyInputStream* is_p) 
{
  for ( ;; ) {
    int src_size;
    const void* src_p;
    if (!is_p->Next(&src_p, &src_size))
      break;
    
    if (!write_to_stream(os_p, src_p, src_size)) {
      throw RuntimeError();
    }
  }
}

static char base16char(char in)
{
  switch(in) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    return in - '0';
  case 'A': case 'B': case 'C':
  case 'D': case 'E': case 'F':
    return (in - 'A') + 10;
  case 'a': case 'b': case 'c':
  case 'd': case 'e': case 'f':
    return in - 'a' + 10;
  default:
    // TODO: ERROR
    return 0;
  }
}

void Lyekka::base16_decode(unsigned char* dest_p, const char* src_p) 
{
  size_t length = strlen(src_p);
  int i;

  assert(length % 2 == 0);

  for (i = 0; i < length / 2; i++) {
    dest_p[i] = 
      base16char(*(src_p + 2*i + 0)) << 4 |
      base16char(*(src_p + 2*i + 1));
  }
}
