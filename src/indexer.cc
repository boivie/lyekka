//  As an example program, we don't want to use any deprecated features
#define BOOST_FILESYSTEM_NO_DEPRECATED

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include <iostream>
#include <sys/stat.h>
#include "indexer.h"

using namespace Lyekka;
using namespace std;

namespace fs = boost::filesystem;
  

void Indexer::update_index(std::string& path)
{
  fs::path full_path( fs::initial_path<fs::path>() );
  full_path = fs::system_complete( fs::path( path ) );

  if (!fs::exists(full_path) || !fs::is_directory( full_path ))
  {
    throw exception();
  }

  std::cout << "\nIn directory: " << full_path.directory_string() << "\n\n";
  sd::sql query(m_db);
  query << "BEGIN TRANSACTION";
  query.step();
  index_path(full_path, 0);
  query << "COMMIT";
  query.step(); 
}

void Indexer::visit_file(fs::directory_iterator& itr, IndexedFile& file)
{
  struct stat statr;
  if (stat(itr->path().string().c_str(), &statr) == 0)
  {
    if (file.get_mtime() != statr.st_mtime) 
    {
      cout << "UPD: " << file.get_basename() << ": " << file.get_mtime() << " <-> " << statr.st_mtime << endl;
    }
  }
}

void Indexer::visit_dir(fs::directory_iterator& itr, IndexedDir& dir)
{
  cout << "Dir: " << dir.get_basename() << endl;
}

void Indexer::visit_other(fs::directory_iterator& itr, IndexedOther& item)
{
  cout << "Other: " << item.get_basename() << endl;
}


FileList Indexer::get_known_files(int dir_db_id)
{ 
  FileList files;
  sd::sql query(m_db);
  query << "SELECT id,name,mtime,ctime,type FROM files WHERE parent = ?";
  query << dir_db_id;
  int id;
  string name;
  int64_t mtime;
  int64_t ctime;
  int type;
  
  while (query.step()) 
  { 
    query >> id >> name >> mtime >> ctime >> type;
    if (type == INDEXED_TYPE_FILE) 
    {
      files.insert(pair<std::string,IndexedBase*>(name, new IndexedFile(id, name, mtime, ctime)));
    }
    else if (type == INDEXED_TYPE_DIR) 
    {
      files.insert(pair<std::string,IndexedBase*>(name, new IndexedDir(id, name, mtime, ctime)));
    }
    else if (type == INDEXED_TYPE_OTHER) 
    {
      files.insert(pair<std::string,IndexedBase*>(name, new IndexedOther(id, name, mtime, ctime)));
    } 
    else 
    {
      assert(0);
    }
  }    
  return files;
}

static IndexedType get_type_from_itr(fs::directory_iterator& itr)
{
  return fs::is_directory(itr->status()) ? INDEXED_TYPE_DIR :
    fs::is_regular_file(itr->status()) ? INDEXED_TYPE_FILE :
    INDEXED_TYPE_OTHER;
}

void Indexer::index_path(fs::path& path, int dir_db_id)
{
  //  cout << "Inside " << path.string() << endl;
  FileList known = get_known_files(dir_db_id);
  std::vector<DirToVisit> subdirs;

  fs::directory_iterator end_iter;
  for (fs::directory_iterator dir_itr(path);
       dir_itr != end_iter;
       ++dir_itr )
  {
    FileList::iterator known_i = known.find(dir_itr->path().filename());
    auto_ptr<IndexedBase> item_p;
    if (known_i != known.end()) 
    {
      item_p.reset(known_i->second);
      known.erase(known_i);
      if (item_p->get_type() != get_type_from_itr(dir_itr)) 
      {
        // Probably a file called "foo" is now a directory.
        delete_obj(*item_p);
        item_p.reset();
      }
    }

    try
    {
      if (fs::is_directory(dir_itr->status()))
      {
        if (!item_p.get())
          item_p = create_dir_obj(dir_itr, dir_db_id);

        subdirs.push_back(DirToVisit(dir_itr->path(), item_p->get_dbid()));
        visit_dir(dir_itr, *(IndexedDir*)item_p.get());
      }
      else if (fs::is_regular_file(dir_itr->status()))
      {
        if (!item_p.get())
          item_p = create_file_obj(dir_itr, dir_db_id);
        visit_file(dir_itr, *(IndexedFile*)item_p.get());
      }
      else
      {
        if (!item_p.get())
          item_p = create_other_obj(dir_itr, dir_db_id);
        visit_other(dir_itr, *(IndexedOther*)item_p.get());
      }
    }
    catch ( const std::exception & ex )
    {
      std::cout << dir_itr->path().filename() << " " << ex.what() << std::endl;
    }
  }

  // Delete files that we haven't seen
  delete_files(known);
  
  for (vector<DirToVisit>::iterator i = subdirs.begin(); i != subdirs.end(); i++)
  {
    index_path((*i).path, (*i).dir_db_id);
  }
}

void Indexer::delete_obj(IndexedBase& obj)
{
  sd::sql insert_query(m_db);
  //  assert(obj.get_type() != INDEXED_TYPE_DIR); // Subdirs have to be handled
  insert_query << "DELETE FROM chunks WHERE file_id = ?";
  insert_query << obj.get_dbid();
  insert_query.step();
}


void Indexer::delete_files(FileList& deletemap) 
{
  for (FileList::iterator delete_i = deletemap.begin(); delete_i != deletemap.end(); delete_i++)
  {
    auto_ptr<IndexedBase> item_p(delete_i->second);
    delete_obj(*item_p);
  }
}

void Indexer::create_db_obj(boost::filesystem::directory_iterator& itr, int type, int parent_id, time_t* mtime_p, time_t* ctime_p)
{
  struct stat statr;
  sd::sql query(m_db);
  if (stat(itr->path().string().c_str(), &statr) == 0)
  {
    string name = itr->path().filename();
    query << "INSERT INTO FILES (parent,name,type,mtime,ctime) VALUES (?, ?, ?, ?, ?)";
    query << parent_id << name << type << statr.st_mtime << statr.st_ctime;
    query.step();
    *mtime_p = statr.st_mtime;
    *ctime_p = statr.st_ctime;
  }
  else  
  {
    cerr << "Failed to get file information for " << itr->path().filename() << " - skipping." << endl;
  }
}

std::auto_ptr<IndexedFile> Indexer::create_file_obj(boost::filesystem::directory_iterator& itr, int parent_id)
{
  time_t mtime, ctime;
  create_db_obj(itr, INDEXED_TYPE_FILE, parent_id, &mtime, &ctime);
  auto_ptr<IndexedFile> ret_p(new IndexedFile(m_db.last_rowid(), itr->path().filename(), mtime, ctime));
  return ret_p;
}

std::auto_ptr<IndexedDir> Indexer::create_dir_obj(boost::filesystem::directory_iterator& itr, int parent_id)
{
  time_t mtime, ctime;
  create_db_obj(itr, INDEXED_TYPE_DIR, parent_id, &mtime, &ctime);
  auto_ptr<IndexedDir> ret_p(new IndexedDir(m_db.last_rowid(), itr->path().filename(), mtime, ctime));
  return ret_p;
}

std::auto_ptr<IndexedOther> Indexer::create_other_obj(boost::filesystem::directory_iterator& itr, int parent_id)
{
  time_t mtime, ctime;
  create_db_obj(itr, INDEXED_TYPE_OTHER, parent_id, &mtime, &ctime);
  auto_ptr<IndexedOther> ret_p(new IndexedOther(m_db.last_rowid(), itr->path().filename(), mtime, ctime));
  return ret_p;
}
