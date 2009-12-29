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

#if defined(HAVE_STAT64) && !defined(__APPLE__)
typedef struct stat64 statbuf;
#define file_stat(f, s) stat64(f, s)
#else
typedef struct stat statbuf;
#define file_stat(f, s) stat(f, s)
#endif

int Indexer::get_root_path_id(boost::filesystem::path& path)
{
  // First make sure that the path exists as a root level path. If not, add it.
  sd::sql query(m_db);
  query << "SELECT id FROM files WHERE parent = 0 AND name = ?";
  query << path.string();

  if (query.step()) {
    int id;
    query >> id;
    return id;
  }
  else 
  {
    query << "INSERT INTO FILES (parent,name,type,mtime,ctime,size) VALUES (0, ?, ?, 0, 0, 0)";
    query << path.string() << INDEXED_TYPE_DIR;
    query.step();
    return m_db.last_rowid();
  }
}

void Indexer::update_index(std::string& path)
{
  fs::path full_path( fs::initial_path<fs::path>() );
  full_path = fs::system_complete( fs::path( path ) );

  if (!fs::exists(full_path) || !fs::is_directory( full_path ))
  {
    throw exception();
  }

  int id = get_root_path_id(full_path);

  m_db << "BEGIN TRANSACTION";
  index_path(full_path, id);
  m_db << "COMMIT TRANSACTION";
}

#define MAX_BLOCK_SIZE (30*1024*1024)
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

void Indexer::make_chunks(fs::directory_iterator& itr, IndexedFile& file, uint64_t size)
{
  // The chunks created will be anonymous at first - they have no cid or key - only an offset and size.
  // They will get their identity when exporting chunks. That's when we encrypt and calculate checksums
  sd::sql delquery(m_db);
  delquery << "DELETE FROM local_mapping WHERE file_id = ?";
  delquery << file.get_dbid();
  delquery.step();
  uint64_t remaining = size;
  uint64_t offset = 0;
  while (remaining > 0)
  {
    uint32_t thispart = MIN(remaining, MAX_BLOCK_SIZE);
    sd::sql insquery(m_db);
    insquery << "INSERT INTO chunks (size, sha, key) VALUES (?, NULL, NULL)";
    insquery << thispart;
    insquery.step();
    insquery << "INSERT INTO local_mapping (file_id, chunk_id, offset) VALUES (?, ?, ?)";
    insquery << file.get_dbid() << m_db.last_rowid() << offset;
    insquery.step();
    remaining -= thispart;
    offset += thispart;
  }
}

void Indexer::visit_file(fs::directory_iterator& itr, IndexedFile& file)
{
  statbuf statr;
  if (file_stat(itr->path().string().c_str(), &statr) == 0)
  {
    if ((file.get_mtime() != statr.st_mtime) ||
      (file.get_ctime() != statr.st_ctime) ||
      (file.get_size() != statr.st_size))
    {
      m_stats.updated++;
      // Chunkify it - will calculate chunks and add to db
      make_chunks(itr, file, statr.st_size);
      // Update file metadata
      //      cout << "UPD: " << file.get_basename() << endl;
      sd::sql query(m_db);
      query << "UPDATE files SET mtime=?, ctime=?, size=? WHERE id=?";  
      query << statr.st_mtime << statr.st_ctime << statr.st_size << file.get_dbid();
      query.step();
    }
  }
}

void Indexer::visit_dir(fs::directory_iterator& itr, IndexedDir& dir)
{
}

void Indexer::visit_other(fs::directory_iterator& itr, IndexedOther& item)
{
  cout << "Other: " << item.get_basename() << endl;
}


FileList Indexer::get_known_files(int dir_db_id)
{ 
  FileList files;
  sd::sql query(m_db);
  query << "SELECT id,name,mtime,ctime,size,type FROM files WHERE parent = ?";
  query << dir_db_id;
  int id;
  string name;
  int64_t mtime;
  int64_t ctime;
  uint64_t size;
  int type;
  
  while (query.step()) 
  { 
    query >> id >> name >> mtime >> ctime >> size >> type;
    if (type == INDEXED_TYPE_FILE) 
    {
      files.insert(pair<std::string,IndexedBase*>(name, new IndexedFile(id, name, mtime, ctime, size)));
    }
    else if (type == INDEXED_TYPE_DIR) 
    {
      files.insert(pair<std::string,IndexedBase*>(name, new IndexedDir(id, name, mtime, ctime, size)));
    }
    else if (type == INDEXED_TYPE_OTHER) 
    {
      files.insert(pair<std::string,IndexedBase*>(name, new IndexedOther(id, name, mtime, ctime, size)));
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
  cout << path.string() << endl;
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
        {
          item_p = create_file_obj(dir_itr, dir_db_id);
          m_stats.added++;
        }
        m_stats.found++;
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
  m_stats.deleted++;
}


void Indexer::delete_files(FileList& deletemap) 
{
  for (FileList::iterator delete_i = deletemap.begin(); delete_i != deletemap.end(); delete_i++)
  {
    auto_ptr<IndexedBase> item_p(delete_i->second);
    delete_obj(*item_p);
  }
}

void Indexer::create_db_obj(boost::filesystem::directory_iterator& itr, int type, int parent_id)
{
  sd::sql query(m_db);
  string name = itr->path().filename();
  query << "INSERT INTO FILES (parent,name,type,mtime,ctime,size) VALUES (?, ?, ?, 0, 0, 0)";
  query << parent_id << name << type << 0 << 0 << 0;
  query.step();
}

std::auto_ptr<IndexedFile> Indexer::create_file_obj(boost::filesystem::directory_iterator& itr, int parent_id)
{
  create_db_obj(itr, INDEXED_TYPE_FILE, parent_id);
  auto_ptr<IndexedFile> ret_p(new IndexedFile(m_db.last_rowid(), itr->path().filename(), 0, 0, 0));
  return ret_p;
}

std::auto_ptr<IndexedDir> Indexer::create_dir_obj(boost::filesystem::directory_iterator& itr, int parent_id)
{
  create_db_obj(itr, INDEXED_TYPE_DIR, parent_id);
  auto_ptr<IndexedDir> ret_p(new IndexedDir(m_db.last_rowid(), itr->path().filename(), 0, 0, 0));
  return ret_p;
}

std::auto_ptr<IndexedOther> Indexer::create_other_obj(boost::filesystem::directory_iterator& itr, int parent_id)
{
  create_db_obj(itr, INDEXED_TYPE_OTHER, parent_id);
  auto_ptr<IndexedOther> ret_p(new IndexedOther(m_db.last_rowid(), itr->path().filename(), 0, 0, 0));
  return ret_p;
}
