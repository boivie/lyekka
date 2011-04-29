#pragma once

#include <map>
#include <google/sparse_hash_map>
#include <string>
#include "chunk_id.h"
#include "pack.h"

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
  ChunkDatabase() : m_chunks(), m_packs() {}

  ChunkFindResult find(const ChunkId& cid);

  void load(const std::string& path) {
    find_indexes(path);
    load_indexes(path);
  }

  void dump_all(void);

  size_t pack_count() const { return m_packs.size(); };
  size_t chunk_count() const { return m_chunks.size(); };

private:
  void load_indexes(const std::string& path);
  void find_indexes(const std::string& path);
  ChunkT m_chunks;
  PackT m_packs;
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
