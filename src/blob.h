#ifndef CHUNKFACTORY_H_INCLUSION_GUARD
#define CHUNKFACTORY_H_INCLUSION_GUARD

#include "chunk.h"
#include <iostream>
#include <google/protobuf/io/zero_copy_stream.h>
#include "sha.h"

namespace Lyekka {
  class Blob {
  public:
    static Blob create(google::protobuf::io::ZeroCopyInputStream* is_p, 
		       google::protobuf::io::ZeroCopyOutputStream* os_p);
    static void unpack(google::protobuf::io::ZeroCopyInputStream* is_p, 
		       google::protobuf::io::ZeroCopyOutputStream* os_p);
    const Sha& hash() { return m_hash; }
  private:
    Sha m_hash;
    Blob() {};
  };

} 

#endif
