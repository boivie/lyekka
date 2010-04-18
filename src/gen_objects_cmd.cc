#include <iostream>
#include "cmd_handler.h"
#include "lyekka.h"
#include "file_system_iterator.h"
#include "file.h"
#include "archive.h"
#include "object_generator.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;

class GenObjectCmdGenerator : public ObjectGenerator {
public:
  GenObjectCmdGenerator(ObjectWriter& dest) : 
    ObjectGenerator(dest) {}

  virtual void on_folder(const ObjectIdentifier& oi, const Tree& tree) {
    ShaBase16Buf buf;
    cout << "T " << oi.sha().base16(buf) << endl;
  }

  virtual void on_blob(const Blob& blob) {
    ShaBase16Buf buf;
    cout << "B " << blob.hash().base16(buf) << endl;
  }
};

static int gen_objects(CommandLineParser& c)
{
  string input;
  string archive = "";
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input directory")
    ("archive-out,a", bpo::value<string>(&archive), "archive");
  c.p.add("input", -1);
  c.parse_options();
  
  FolderPtr folder_p = FileSystemIterator().iterate(input);
  if (archive == "") {
    FileWriter fw;
    GenObjectCmdGenerator og(fw);
    og.generate(folder_p);
  } else {
    ArchiveWriter aw(archive);
    GenObjectCmdGenerator og(aw);
    auto_ptr<ObjectIdentifier> oi_p = og.generate(folder_p);
    aw.set_entry_point(oi_p->sha());
  }

  return 0;
}

LYEKKA_COMMAND(gen_objects, "gen-objects", "", "Generates objects");

