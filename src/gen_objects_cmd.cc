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
#include "lyekka.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "file_system_iterator.h"

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;
namespace fs = boost::filesystem;
using namespace boost;
namespace bpo = boost::program_options;


static Sha visit_folder(FolderPtr f)
{
  char buf[256/4 + 1];
  TreeBuilder tb;
  FolderList folders = f->get_sub_folders();
  for (FolderList::iterator child_i = folders.begin(); 
       child_i != folders.end();
       ++child_i) {
    pb::TreeRef* tr_p = tb.add_tree();
    tr_p->MergeFrom((*child_i)->pb());
    tr_p->SetExtension(pb::tree_sha_ext, visit_folder(*child_i).mutable_string());
  }

  FileList files = f->get_files();
  for (FileList::iterator child_i = files.begin();
       child_i != files.end();
       ++child_i) {
    FilePtr file_p = *child_i;
    pb::FileEntry* fe_p = tb.add_file();
    fe_p->MergeFrom(file_p->pb());

    int in_fd = xopen(file_p->path().string().c_str(), O_RDONLY);
    PartList parts = file_p->get_parts();
    for (PartList::iterator part_i = parts.begin();
	 part_i != parts.end();
	 ++part_i) {
      PartPtr part_p = *part_i;
      FileOutputStream fos(xopen("_blob.tmp", O_WRONLY | O_CREAT | O_TRUNC, 0644));
      fos.SetCloseOnDelete(true);
      Blob blob = Blob::create_from_fd(in_fd, part_p->offset(), part_p->size(), &fos);
      cout << "B " << blob.hash().base16(buf) << endl;
      pb::Part* pt_p = fe_p->add_parts();
      pt_p->set_offset(part_p->offset());
      pt_p->set_size(part_p->size());
      pt_p->SetExtension(pb::part_sha_ext, blob.hash().mutable_string());
      rename("_blob.tmp", buf);
    }
    close(in_fd);
  }

  shared_ptr<Tree> tree_p = tb.build();
  FileOutputStream fos(xopen("_tree.tmp", O_WRONLY | O_CREAT | O_TRUNC, 0644));
  fos.SetCloseOnDelete(true);
  tree_p->serialize(&fos);
  cout << "T " << tree_p->get_sha().base16(buf) << endl;
  rename("_tree.tmp", buf);
  return tree_p->get_sha();
}

static int gen_objects(CommandLineParser& c)
{
  string input;
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file");
  c.p.add("input", -1);
  c.parse_options();
  
  FolderPtr folder_p = FileSystemIterator().iterate(input);
  visit_folder(folder_p);

  return 0;
}

LYEKKA_COMMAND(gen_objects, "gen-objects", "", "Generates");

