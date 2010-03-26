#include <fcntl.h>
#include <iostream>
#include <string>
#include <fstream>
#include <list>
#include <ctime>
#include "foldermap.h"
#include "db.h"
#include "cmd_handler.h"
#include <sys/stat.h>
#include "blob.h"
#include "tree.h"
#include "lyekka_impl.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;
namespace fs = boost::filesystem;
using namespace boost;

static void walker(Folder& f)
{
  // Get a list of files, and their chunks.
  sd::sqlite& db = Db::get();
  sd::sql query(db);  
  query << "SELECT f.id, f.name, f.size, f.mtime, m.offset, c.size " 
    "FROM files f, parts m, objects c " 
    "WHERE m.file_id = f.id AND m.object_id = c.id AND f.parent = ? " 
    "ORDER BY f.id, m.offset";
  query << f.id;
  TreeBuilder tb;
  int64_t last_file_id = -1;
  pb::FileEntry* fe_p = NULL;
  while (query.step())
  { 
    string filename;
    uint64_t offset;
    uint32_t size;
    uint64_t file_size;
    int64_t file_id;
    time_t mtime;
    query >> file_id >> filename >> file_size >> mtime >> offset >> size;
    fs::path p = fs::path(f.name) / fs::path(filename);
    int in_fd = open(p.string().c_str(), O_RDONLY);
    FileOutputStream fos(open("_blob.tmp", O_WRONLY | O_CREAT | O_TRUNC, 0755));
    fos.SetCloseOnDelete(true);
    Blob blob = Blob::create_from_fd(in_fd, offset, size, &fos);
    char buf[256/4 + 1];
    cout << "B " << blob.hash().base16(buf) << endl;
    rename("_blob.tmp", buf);
    if (file_id != last_file_id) {
      fe_p = tb.add_file();
      last_file_id = file_id;
      fe_p->set_name(filename);
      fe_p->set_mode(0755);
      fe_p->set_size(file_size);
      fe_p->set_mtime(mtime);
    }
    pb::Part* part_p = fe_p->add_parts();
    part_p->set_offset(offset);
    part_p->set_size(size);
    part_p->SetExtension(pb::part_sha_ext, buf);
  }
  shared_ptr<Tree> tree_p = tb.build();
  
  int fd = open("_tree.tmp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  FileOutputStream fos(fd);
  fos.SetCloseOnDelete(true);
  tree_p->serialize(&fos);

  char base16[256/4 + 1];
  cout << "T " << tree_p->get_sha().base16(base16) << endl;
  rename("_tree.tmp", base16);
}


static int gen_objects(CommandLineParser& c)
{
  c.parse_options();
  FolderMap fm;
  fm.load();
  fm.walk_post_order(walker); 
  return 0;
}

LYEKKA_COMMAND(gen_objects, "gen-objects", "", "Generates");

