#ifndef CHUNK_H_INCLUSION_GUARD
#define CHUNK_H_INCLUSION_GUARD

#include <stdint.h>
#include <vector>
#include <assert.h>


#define SHA_SIZE_BITS 256
#define SHA_SIZE_BYTES (SHA_SIZE_BITS / 8)
#define SHA_SIZE_B16  (SHA_SIZE_BITS / 4)
namespace Lyekka {

class Chunk 
{
public:
  Chunk(uint8_t* sha_p, uint8_t* key_p, size_t offset, size_t orig_size)
    : m_offset(offset), m_orig_size(orig_size)
  {
    memcpy(m_sha, sha_p, SHA_SIZE_BYTES);
    memcpy(m_key, key_p, SHA_SIZE_BYTES);
  }
  
  size_t get_orig_size() { return m_orig_size; }
  uint64_t get_offset() { return m_offset; }
  void get_sha(uint8_t* sha_p) { memcpy(sha_p, m_sha, SHA_SIZE_BYTES); }
  void get_key(uint8_t* key_p) { memcpy(key_p, m_key, SHA_SIZE_BYTES); }

 private:
  size_t m_orig_size;
  uint64_t m_offset;
  uint8_t m_key[SHA_SIZE_BYTES];
  uint8_t m_sha[SHA_SIZE_BYTES];
};

}


#endif
