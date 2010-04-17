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
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include "lyekka.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "archive.h"
#include "file.h"

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;
namespace fs = boost::filesystem;
namespace bpo = boost::program_options;
using namespace boost;

static void unpack_file(const fs::path& cwd, const ObjectReader& rdr, 
			const Tree& tree, const pb::FileEntry& fe) 
{
  const fs::path file = cwd / fs::path(fe.name());
  FileOutputStream fos(xopen(file.string().c_str(), O_WRONLY | O_CREAT | O_TRUNC, fe.mode()));
  fos.SetCloseOnDelete(true);
  // TODO: Verify that all parts are present.
  for (int i = 0; i < fe.parts_size(); i++) {
    const pb::Part& pt = fe.parts(i);
    shared_ptr<ObjectInputStream> obj_p = rdr.find(tree.get_ref(pt.sha_idx()));
    Blob::unpack(obj_p.get(), &fos, NULL);
  }
  fs::last_write_time(file, (time_t)fe.mtime());
  cout << file << " [OK]" << endl;
}

static void unpack_tree(const fs::path& cwd, const ObjectReader& rdr, 
			const Sha& sha) 
{
  shared_ptr<ObjectInputStream> obj_p = rdr.find(sha);
  auto_ptr<Tree> tree_p;
  try {
    tree_p = Tree::deserialize(obj_p.get());
  } catch (InvalidTreeException& e) {
    cerr << e.what() << endl;
    exit(1);
  }
  const pb::Tree& pb = tree_p->get_pb();
  // Visit directories
  for (int i = 0; i < pb.subdirs_size(); i++) {
    const pb::TreeRef& tr = pb.subdirs(i);
    const fs::path dir_path = cwd / tr.name();
    if (!fs::create_directory(dir_path)) {
      cerr << "Failed to create " << dir_path << endl;
      throw RuntimeError();
    }
    cout << dir_path << endl;
    unpack_tree(dir_path, rdr, tree_p->get_ref(tr.sha_idx()));
    fs::last_write_time(dir_path, tr.mtime());
    // TODO: Set permission
  }
  // Visit files.
  for (int i = 0; i < pb.files_size(); i++) {
    unpack_file(cwd, rdr, *tree_p, pb.files(i));
  }
}

static int unpack(CommandLineParser& c)
{
  string input;
  string archive;
  string output = ".";
  c.po.add_options()
    ("input-file,i", bpo::value<string>(&input), "input file")
    ("input-archive,a", bpo::value<string>(&archive), "input archive")
    ("output,o", bpo::value<string>(&output), "output dir");
  c.parse_options();

  if (archive != "") {
    const fs::path p(archive);
    ArchiveReader ar(p);
    unpack_tree(fs::path(output), ar, ar.entry_point());
  } else {
    FileReader fr;
    Sha sha;
    sha.set_base16(input.c_str());
    unpack_tree(fs::path(output), fr, sha);
  }

  return 0;
}

LYEKKA_COMMAND(unpack, "unpack", "", "Unpacks an archive, a tree or similar");

