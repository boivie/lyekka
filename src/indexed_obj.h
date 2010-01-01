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
  int64_t get_size() { return m_size; }
  const std::string& get_basename(void) { return m_basename; }
  
protected: 
  IndexedBase(int dbid, std::string basename, time_t mtime, time_t ctime, uint64_t size, IndexedType type) : 
    m_dbid(dbid), m_basename(basename), m_mtime(mtime), m_ctime(ctime), m_size(size), m_type(type) {};
  std::string m_basename;
  uint64_t m_size;
  int m_dbid;
  IndexedType m_type;
  time_t m_mtime;
  time_t m_ctime;
};

class IndexedFile : public IndexedBase 
{
public:
  IndexedFile(int dbid, std::string basename, time_t mtime, time_t ctime, uint64_t size) 
    : IndexedBase(dbid, basename, mtime, ctime, size, INDEXED_TYPE_FILE) { };
};

class IndexedDir : public IndexedBase 
{
public:
  IndexedDir(int dbid, std::string basename, time_t mtime, time_t ctime) 
    : IndexedBase(dbid, basename, mtime, ctime, 0, INDEXED_TYPE_DIR) { };

};

}

#endif

