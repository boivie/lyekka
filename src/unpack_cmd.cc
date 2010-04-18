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
    if (pt.has_key()) {
      const uint8_t* key_buf = (const uint8_t*)pt.key().c_str();
      Aes128Key key(key_buf, key_buf + 128/8);
      Blob::unpack(obj_p.get(), &fos, &key);
    } else {
      Blob::unpack(obj_p.get(), &fos, NULL);
    }
  }
  fs::last_write_time(file, (time_t)fe.mtime());
  cout << file << " [OK]" << endl;
}

static void unpack_tree(const fs::path& cwd, const ObjectReader& rdr, 
			const ObjectIdentifier& oi) 
{
  shared_ptr<ObjectInputStream> obj_p = rdr.find(oi.sha());
  auto_ptr<const Tree> tree_p;
  try {
    if (oi.has_key()) {
      cerr << "unpacking " << oi.sha() << " using " << oi.key().base16() << endl;
      tree_p = Tree::deserialize(obj_p.get(), &oi.key());
    } else {
      tree_p = Tree::deserialize(obj_p.get());
    }
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
    Sha sha = tree_p->get_ref(tr.sha_idx());
    if (tr.has_key()) {
      const uint8_t* key_buf = (const uint8_t*)tr.key().c_str();
      Aes128Key key(key_buf, key_buf + 128/8);
      auto_ptr<ObjectIdentifier> oi_p(new Aes128ObjectIdentifier(sha, key));
      unpack_tree(dir_path, rdr, *oi_p);
    } else {
      auto_ptr<ObjectIdentifier> oi_p(new ObjectIdentifier(sha));
      unpack_tree(dir_path, rdr, *oi_p);
    }
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
  string input = "";
  string archive;
  string output = ".";
  string key = "";
  c.po.add_options()
    ("input-file,i", bpo::value<string>(&input), "input file")
    ("input-archive,a", bpo::value<string>(&archive), "input archive")
    ("output,o", bpo::value<string>(&output), "output dir")
    ("key,K", bpo::value<string>(&key), "encryption key");
  c.parse_options();

  auto_ptr<ObjectReader> or_p;
  Sha sha;
  if (archive != "") {
    ArchiveReader* ar_p = new ArchiveReader(fs::path(archive));
    sha = ar_p->entry_point();
    or_p.reset(ar_p);
  } else {
    or_p.reset(new FileReader());
    sha.set_base16(input.c_str());
  }

  auto_ptr<ObjectIdentifier> oi_p;
  if (key == "") {
    oi_p.reset(new ObjectIdentifier(sha));
  } else {
    uint8_t key_buf[128/8];
    uint8_t iv_buf[128/8];
    base16_decode(key_buf, key.c_str(), 128/4);
    base16_decode(iv_buf, key.c_str() + 128/4, 128/4);
    Aes128Key a_key(key_buf, iv_buf);
    oi_p.reset(new Aes128ObjectIdentifier(sha, a_key));
  }

  unpack_tree(fs::path(output), *or_p, *oi_p);

  return 0;
}

LYEKKA_COMMAND(unpack, "unpack", "", "Unpacks an archive, a tree or similar");

