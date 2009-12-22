#ifndef CHUNK_H_INCLUSION_GUARD
#define CHUNK_H_INCLUSION_GUARD

#define CID_SIZE_BITS 256
#define CID_SIZE_BYTES (CID_SIZE_BITS / 8)
#define CID_SIZE_B16  (CID_SIZE_BITS / 4)

class Chunk 
{
  Chunk();
  Chunk(std::vector<uint8_t>& cid, std::vector<uint8_t>& key, size_t offset, size_t orig_size)
    : m_cid(cid), m_key(key), m_offset(offset), m_orig_size(orig_size)
  {
    assert(cid.size() == CID_SIZE_BYTES && key.size() == CID_SIZE_BYTES);
  }
  
  size_t get_orig_size() { return m_orig_size; }
  size_t get_offset() { return m_offset; }
  std::vector<uint8_t>& get_cid(void) { return m_cid; }
  std::vector<uint8_t>& get_key(void) { return m_key; }

 private:
  size_t m_orig_size;
  size_t m_offset;
  std::vector<uint8_t> m_key;
  std::vector<uint8_t> m_cid;
};


#endif
