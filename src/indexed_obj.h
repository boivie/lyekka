#ifndef INDEXED_OBJ_H_INCLUSION_GUARD
#define INDEXED_OBJ_H_INCLUSION_GUARD

#include <string>
#include <vector>
#include "sdsqlite/sdsqlite.h"

namespace Lyekka 
{

  typedef enum { INDEXED_TYPE_FILE, INDEXED_TYPE_DIR, INDEXED_TYPE_OTHER } IndexedType;

class IndexedBase 
{
public:
  int get_dbid() { return m_dbid; }
  IndexedType get_type() { return m_type; }
  time_t get_mtime() { return m_mtime; }
  time_t get_ctime() { return m_ctime; }
  const std::string& get_basename(void) { return m_basename; }
  
protected: 
  IndexedBase(int dbid, std::string basename, time_t mtime, time_t ctime, IndexedType type) : 
    m_dbid(dbid), m_basename(basename), m_mtime(mtime), m_ctime(ctime), m_type(type) {};
  std::string m_basename;
  
  int m_dbid;
  IndexedType m_type;
  time_t m_mtime;
  time_t m_ctime;
};

class IndexedFile : public IndexedBase 
{
public:
  IndexedFile(int dbid, std::string basename, time_t mtime, time_t ctime) 
    : IndexedBase(dbid, basename, mtime, ctime, INDEXED_TYPE_FILE) { };
  
  const std::vector<Chunk>& get_chunks() { return m_chunks; }
  const void set_chunks(std::vector<Chunk>& chunks) { m_chunks = chunks; }
  
protected:
  std::vector<Chunk> m_chunks;
};

class IndexedDir : public IndexedBase 
{
public:
  IndexedDir(int dbid, std::string basename, time_t mtime, time_t ctime) 
    : IndexedBase(dbid, basename, mtime, ctime, INDEXED_TYPE_DIR) { };

};

class IndexedOther : public IndexedBase 
{
public:
  IndexedOther(int dbid, std::string basename, time_t mtime, time_t ctime) 
    : IndexedBase(dbid, basename, mtime, ctime, INDEXED_TYPE_OTHER) { };

};
}

#endif

