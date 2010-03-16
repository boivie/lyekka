#ifndef HASH_STREAM_H_INCLUSION_GUARD
#define HASH_STREAM_H_INCLUSION_GUARD

#include <vector>
#include <string>
#include <google/protobuf/io/zero_copy_stream.h>
#include "sha2/sha2.h"

namespace Lyekka {

  class Sha256Digest {
  public:
    uint8_t m_digest[SHA256_DIGEST_LENGTH];
    std::vector<uint8_t> get();
    std::string hex();
  };

  class Sha256OutputStream : public google::protobuf::io::ZeroCopyOutputStream {
  public:
    explicit Sha256OutputStream(ZeroCopyOutputStream* sub_stream);
    virtual ~Sha256OutputStream() {}
    
    Sha256Digest get_digest();
    
    // implements ZeroCopyOutputStream ---------------------------------
    bool Next(void** data, int* size);
    void BackUp(int count);
    google::protobuf::int64 ByteCount() const;

  private:
    SHA256_CTX m_ctx;
    void* m_last_data;
    int m_last_size;
    ZeroCopyOutputStream* m_substream;
  };

}
#endif
