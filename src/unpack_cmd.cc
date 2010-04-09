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
#include "mmap_stream.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;
namespace fs = boost::filesystem;
namespace bpo = boost::program_options;
using namespace boost;

static void unpack_part(FileOutputStream& fos, const Tree& tree, const pb::Part& pt) 
{
  char buf[SHA_BITS / 4 + 1];
  int fd = xopen(tree.get_ref(pt.sha_idx()).base16(buf), O_RDONLY);
  MmapInputStream mmis(fd, 0, pt.size());
  cout << "Unpacking blob " << buf << " = " << fd << endl;
  Blob::unpack(&mmis, &fos);
  close(fd);
}

static void unpack_file(const fs::path& cwd, const Tree& tree, const pb::FileEntry& fe) 
{
  fs::path file = cwd / fs::path(fe.name());
  FileOutputStream fos(xopen(file.string().c_str(), O_WRONLY | O_CREAT | O_TRUNC, fe.mode()));
  fos.SetCloseOnDelete(true);
  // TODO: Verify that all parts are present.
  for (int i = 0; i < fe.parts_size(); i++) {
    unpack_part(fos, tree, fe.parts(i));
  }
  fs::last_write_time(file, (time_t)fe.mtime());
  cout << file << " [OK]" << endl;
}

static void unpack_tree(const fs::path& cwd, const Sha& sha) 
{
  char base16[SHA_BITS/4+1];
  FileInputStream fis(xopen(sha.base16(base16), O_RDONLY));
  fis.SetCloseOnDelete(true);
  Tree tree;
  tree.deserialize(&fis);
  const pb::Tree& pb = tree.get_pb();
  // Visit directories
  for (int i = 0; i < pb.subdirs_size(); i++) {
    const pb::TreeRef& tr = pb.subdirs(i);
    fs::path dir_path = cwd / tr.name();
    if (!fs::create_directory(dir_path)) {
      cerr << "Failed to create " << dir_path << endl;
      throw RuntimeError();
    }
    cout << dir_path << endl;
    unpack_tree(dir_path, tree.get_ref(tr.sha_idx()));
    fs::last_write_time(dir_path, tr.mtime());
    // TODO: Set permission
  }
  // Visit files.
  for (int i = 0; i < pb.files_size(); i++) {
    unpack_file(cwd, tree, pb.files(i));
  }
}

static int unpack(CommandLineParser& c)
{
  string input;
  string output = ".";
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file")
    ("output,o", bpo::value<string>(&output), "output dir");
  c.parse_options();
  Sha sha;
  sha.set_base16(input.c_str());
  unpack_tree(fs::path(output), sha);

  return 0;
}

LYEKKA_COMMAND(unpack, "unpack", "", "Unpacks an archive, a tree or similar");

