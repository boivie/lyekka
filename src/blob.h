#ifndef CHUNKFACTORY_H_INCLUSION_GUARD
#define CHUNKFACTORY_H_INCLUSION_GUARD

#include "chunk.h"
#include <iostream>

namespace Lyekka {
  class Blob {
  public:
    unsigned char hash[256/8];
    unsigned char key[256/8];
    static Blob create_from_fd(int in_fd, uint64_t offset, uint32_t size, int out_fd);
    static void unpack_from_fd(int in_fd, uint32_t size, int out_fd);
  private:
    Blob() {};
  };

} 

#endif
