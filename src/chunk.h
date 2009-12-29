#ifndef CHUNK_H_INCLUSION_GUARD
#define CHUNK_H_INCLUSION_GUARD

#include <stdint.h>
#include <vector>
#include <assert.h>


#define SHA_SIZE_BITS 256
#define SHA_SIZE_BYTES (SHA_SIZE_BITS / 8)
#define SHA_SIZE_B16  (SHA_SIZE_BITS / 4)
namespace Lyekka {

  class ChunkHash {
  public:
    ChunkHash() : m_hash(SHA_SIZE_BYTES, '-') {}
    ChunkHash(const std::vector<uint8_t> init) { assert(init.size() == SHA_SIZE_BYTES); m_hash = init; }
    ChunkHash(uint8_t* hash_p) { m_hash.resize(SHA_SIZE_BYTES); memcpy(&m_hash[0], hash_p, SHA_SIZE_BYTES); }
    const std::vector<uint8_t>& get_hash() const { return m_hash; }
  private:
    std::vector<uint8_t> m_hash;
  };

class Chunk 
{
public:
  Chunk(const ChunkHash& sha, const ChunkHash& key, size_t offset, size_t orig_size)
    : m_sha(sha), m_key(key), m_offset(offset), m_orig_size(orig_size)
  {
  }
  
  size_t get_orig_size() { return m_orig_size; }
  uint64_t get_offset() { return m_offset; }
  const ChunkHash& get_sha() const { return m_sha; }
  const ChunkHash& get_key() const { return m_key; }

 private:
  size_t m_orig_size;
  uint64_t m_offset;
  ChunkHash m_key;
  ChunkHash m_sha;
};


}


#endif
