#ifndef INDEXER_H_INCLUSION_GUARD
#define INDEXER_H_INCLUSION_GUARD

#include "sdsqlite/sdsqlite.h"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "chunk.h"
#include "indexed_obj.h"
#include <memory>
#include <map>
#include <string>

namespace Lyekka 
{
  typedef std::map<std::string,IndexedBase*> FileList;

class Indexer 
{
public:
  Indexer(sd::sqlite& db) : m_db(db) { };
  
  void update_index(std::string& path);

private:
  void index_path(boost::filesystem::path& path, int dir_id);
  FileList get_known_files(int dir_db_id);
  std::auto_ptr<IndexedBase> get_obj(boost::filesystem::directory_iterator& itr);
  void visit_file(boost::filesystem::directory_iterator& itr, IndexedFile& file);
  void visit_dir(boost::filesystem::directory_iterator& itr, IndexedDir& dir);
  void visit_other(boost::filesystem::directory_iterator& itr, IndexedOther& item); 
  void delete_files(FileList& filelist);
  void delete_obj(IndexedBase& item);
  std::auto_ptr<IndexedFile> create_file_obj(boost::filesystem::directory_iterator& itr, int parent_id);
  std::auto_ptr<IndexedDir> create_dir_obj(boost::filesystem::directory_iterator& itr, int parent_id);
  std::auto_ptr<IndexedOther> create_other_obj(boost::filesystem::directory_iterator& itr, int parent_id);
  void create_db_obj(boost::filesystem::directory_iterator& itr, int type, int parent_id);
  void make_chunks(boost::filesystem::directory_iterator& itr, IndexedFile& file, uint64_t size);
  int get_root_path_id(boost::filesystem::path& path);

  sd::sqlite& m_db;

  class DirToVisit 
  {
  public:
    DirToVisit(boost::filesystem::path p, int db_id) :
      path(p), dir_db_id(db_id) {};
    boost::filesystem::path path;
    int dir_db_id;
  };
  std::vector<DirToVisit> m_dirs_to_visit;
};

}


#endif
