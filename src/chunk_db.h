#pragma once

#include <map>
#include <google/sparse_hash_map>
#include <boost/filesystem.hpp>
#include <boost/smart_ptr.hpp>
#include <string>
#include <stdint.h>
#include "chunk_id.h"
#include "pack.h"
#include "sha1.h"

class hash_fun2 {
public:
  size_t operator()(const ChunkId& key_value) const {
     return key_value.hash();
   }
};

typedef boost::shared_ptr<Pack> PackPtr;
typedef std::map<long long, PackPtr > PackT;
typedef google::sparse_hash_map< ChunkId, ChunkLocation, hash_fun2 > ChunkT;

class StoredChunk;

class ChunkDatabase {
public:
  ChunkDatabase() : m_chunks(), m_packs() { SHA1_Init(&sha1_ctx); }

  StoredChunk find(const ChunkId& cid);

  void set_path(boost::filesystem::path& path);
  bool load();
  bool repair();
  void dump_all(void);
  bool create_partial();
  void close_partial();
  void write_chunk(StoredChunk& sc);

private:
  const boost::filesystem::path& partial() const { return m_partial; }
  void load_indexes();
  void find_indexes();
  bool has_partial();
  void preallocate(int fd);
  size_t write_data(const void* data_p, size_t len);
  boost::filesystem::path get_idx(const boost::filesystem::path& pack_fname) const;
  boost::filesystem::path m_path;
  boost::filesystem::path m_partial;
  ChunkT m_chunks;
  PackT m_packs;

  int m_partial_idx_fd;
  int m_partial_r_fd;
  int m_partial_w_fd;

  SHA1_CTX sha1_ctx;
  uint64_t m_latest_pack;
  uint32_t m_pack_offset;
};

typedef boost::shared_ptr< std::vector<uint8_t> > BufferPtr;

class StoredChunk {
public:
  StoredChunk(const ChunkId& cid, BufferPtr buffer,
	      size_t payload_offset, size_t payload_size)
    : m_cid(cid), m_buffer(buffer),
      m_payload_offset(payload_offset), m_payload_size(payload_size) {}
  const ChunkId& cid() const { return m_cid; }
  BufferPtr raw_buffer() const { return m_buffer; }
  const size_t payload_offset() const { return m_payload_offset; }
  const size_t payload_size() const { return m_payload_size; }
private:
  const ChunkId m_cid;
  BufferPtr m_buffer;
  size_t m_payload_offset;
  size_t m_payload_size;
};

class ChunkNotFound : public std::exception {

};
