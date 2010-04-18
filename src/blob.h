#ifndef CHUNKFACTORY_H_INCLUSION_GUARD
#define CHUNKFACTORY_H_INCLUSION_GUARD

#include "chunk.h"
#include <iostream>
#include <google/protobuf/io/zero_copy_stream.h>
#include "sha.h"
#include "aes_stream.h"
#include "object.h"
#include <memory>

namespace Lyekka {
  class Blob {
  public:
    static std::auto_ptr<ObjectIdentifier>
    create(google::protobuf::io::ZeroCopyInputStream* is_p, 
	   google::protobuf::io::ZeroCopyOutputStream* os_p,
	   const AesKey* key_p = NULL);
    static void unpack(google::protobuf::io::ZeroCopyInputStream* is_p, 
		       google::protobuf::io::ZeroCopyOutputStream* os_p,
		       const AesKey* key_p = NULL);
    static std::auto_ptr<AesKey> generate_key(google::protobuf::io::ZeroCopyInputStream* is_p);
  private:
    Blob() {};
  };

} 

#endif
