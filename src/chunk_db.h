#pragma once

#include <map>
#include <google/sparse_hash_map>
#include <boost/filesystem.hpp>
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

typedef std::map<long long, Pack> PackT;
typedef google::sparse_hash_map< ChunkId, ChunkLocation, hash_fun2 > ChunkT;

class ChunkFindResult;

class ChunkDatabase {
public:
  ChunkDatabase() : m_chunks(), m_packs() { SHA1_Init(&sha1_ctx); }

  ChunkFindResult find(const ChunkId& cid);

  void set_path(boost::filesystem::path& path);
  bool load();
  bool repair();
  void dump_all(void);
  void create_partial();
  void close_partial();
  void write_chunk(const ChunkId& cid, const void* data, size_t len);

private:
  const boost::filesystem::path& partial() const { return m_partial; }
  void load_indexes();
  void find_indexes();
  bool has_partial();
  void preallocate(int fd);
  size_t write_data(const void* data_p, size_t len);
  boost::filesystem::path m_path;
  boost::filesystem::path m_partial;
  ChunkT m_chunks;
  PackT m_packs;
  int m_pack_fd;
  int m_index_fd;
  SHA1_CTX sha1_ctx;
  uint64_t m_latest_pack;
  uint32_t m_pack_offset;
};

class ChunkFindResult {
public:
  ChunkFindResult(ChunkDatabase db, const Pack& pack, const ChunkLocation& location) : m_db(db), m_pack(pack), m_location(location) {};
  const Pack& pack(void) const { return m_pack; }
  uint32_t offset(void) const { return m_location.offset(); }
  uint32_t size(void) const { return m_location.size(); }
private:
  const ChunkDatabase& m_db;
  const Pack& m_pack;
  const ChunkLocation& m_location;
};
