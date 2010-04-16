#ifndef CHUNKFACTORY_H_INCLUSION_GUARD
#define CHUNKFACTORY_H_INCLUSION_GUARD

#include "chunk.h"
#include <iostream>
#include <google/protobuf/io/zero_copy_stream.h>
#include "sha.h"
#include "aes_stream.h"
#include "boost/shared_ptr.hpp"

namespace Lyekka {
  class Blob {
  public:
    static Blob create(google::protobuf::io::ZeroCopyInputStream* is_p, 
		       google::protobuf::io::ZeroCopyOutputStream* os_p,
		       const AesKey* key_p);
    static void unpack(google::protobuf::io::ZeroCopyInputStream* is_p, 
		       google::protobuf::io::ZeroCopyOutputStream* os_p,
		       const AesKey* key_p);
    static boost::shared_ptr<AesKey> generate_key(google::protobuf::io::ZeroCopyInputStream* is_p);

    const Sha& hash() const { return m_hash; }
  private:
    Sha m_hash;
    Blob() {};
  };

} 

#endif
