#include <iostream>
#include <sys/mman.h>
#include <errno.h>
#include <google/protobuf/io/zero_copy_stream.h>

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

