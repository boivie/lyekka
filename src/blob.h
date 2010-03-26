#ifndef CHUNKFACTORY_H_INCLUSION_GUARD
#define CHUNKFACTORY_H_INCLUSION_GUARD

#include "chunk.h"
#include <iostream>
#include <google/protobuf/io/zero_copy_stream.h>
#include "sha.h"

namespace Lyekka {
  class Blob {
  public:
    static Blob create_from_fd(int in_fd, uint64_t offset, uint32_t size, google::protobuf::io::ZeroCopyOutputStream* os_p);
    static void unpack_from_fd(int in_fd, uint32_t size, google::protobuf::io::ZeroCopyOutputStream* os_p);
    const Sha& hash() { return m_hash; }
    const Sha& key() { return m_key; }
  private:
    void init_from_mem(const void* mem_p, uint32_t size, google::protobuf::io::ZeroCopyOutputStream* os_p);
  
    Sha m_hash;
    Sha m_key;
    Blob() {};
  };

} 

#endif
