#include <iostream>
#include <string>
#include <list>
#include "cmd_handler.h"
#include "foldermap.h"
#include "db.h"
#include "sdsqlite/sdsqlite.h"
#include "main.pb.h"
#include "hash_stream.h"
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <arpa/inet.h>
#include <fstream>

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;

class File {
public:
  int64_t id;
  string name;
  int64_t mtime; 
  int64_t ctime;
  uint64_t size;
};

bool compare_files(const File& lhs, const File& rhs)
{
  return lhs.name < rhs.name;
}

bool compare_folders(const Folder* lhs_p, const Folder* rhs_p)
{
  return lhs_p->name < rhs_p->name;
}

static void walker(Folder& f)
{
  cout << f.name << endl;
  list<Folder*> folders;
  list<File> files;
  // Get a list of trees  
  // TODO: Filter out they who don't have a chunk generated - because they are "dead references"
  for (Folder* child_p = f.first_child_p; child_p != NULL; child_p = child_p->sibling_p)
    if (child_p->has_chunk)
      folders.push_back(child_p);
  // Get a list of files, and their chunks.
  sd::sqlite& db = Db::get();
  sd::sql query(db);  
  query << "SELECT f.id, f.name, f.mtime, f.ctime, f.size " 
    "FROM files f, file_mapping m, chunks c " 
    "WHERE m.file_id = f.id AND m.chunk_id = c.id AND c.sha IS NOT NULL AND f.parent = ?";
  query << f.id;
  while (query.step())
  { 
    File file;
    int chunks;
    query >> file.id >> file.name >> file.mtime >> file.ctime >> file.size;
    files.push_back(file);
  }
 
  ofstream tmpfile("tree.tmp", ios_base::out | ios_base::binary);
  std::list< vector<uint8_t> > tree_chunks;
  std::list< vector<uint8_t> > file_chunks;

  if (!files.empty() || !folders.empty())
  {
    // Sort them both and generate a protobuf, which will be saved.
    files.sort(compare_files);
    folders.sort(compare_folders);
    
    pb::Tree pb;
    for (list<File>::iterator i = files.begin(); i != files.end(); ++i)
    {
      pb::FileEntry* fe_p = pb.add_files();
      fe_p->set_name(i->name);
      fe_p->set_mode(0777);
      fe_p->set_mtime(i->mtime);
      fe_p->set_ctime(i->ctime);
      fe_p->set_size(i->size);
      // Add chunks.  
      sd::sql cquery(db); 
      cquery << "SELECT m.offset, c.size, c.sha, c.key FROM file_mapping m, chunks c WHERE m.file_id=? AND c.id=m.chunk_id ORDER BY m.offset";
      cquery << i->id;
      std::vector<uint8_t> sha;
      std::vector<uint8_t> key;      
      uint64_t offset;
      uint32_t size;
      while (cquery.step())
      {
        cquery >> offset >> size >> sha >> key;
        pb::Chunk* c_p = fe_p->add_chunks();
        c_p->set_offset(offset);
        c_p->set_size(size);
        c_p->set_sha(&sha[0], sha.size());
        c_p->set_key(&key[0], key.size());  
        file_chunks.push_back(sha);
      }
    }
    // Generate FREF part.
    tmpfile.write("FREF", 4); 
    file_chunks.sort();
    uint32_t fref_size = htonl(file_chunks.size() * (256/8));
    tmpfile.write((char*)&fref_size, 4);
    for (list<vector<uint8_t> >::const_iterator i = file_chunks.begin(); i != file_chunks.end(); ++i)
    {
      const std::vector<uint8_t>& item = *i;
      tmpfile.write((char*)&item[0], item.size());
    }

    string out;
    StringOutputStream stros(&out); 
    Sha256OutputStream hashos(&stros);
    pb.SerializeToZeroCopyStream(&hashos);

    // Save the hash in the database
    
  }
}

static int generate_index(CommandLineParser& c)
{
  c.parse_options();
  FolderMap fm;
  fm.load();
  cout << "begin" << endl;
  fm.walk_post_order(walker); 
  cout << "end" << endl;

  return 0;
}

LYEKKA_COMMAND(generate_index, "generate-index", "", "Generates a snapshot index from the current database state");

