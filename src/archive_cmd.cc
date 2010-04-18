#include <boost/filesystem.hpp>
#include <iomanip>
#include <iostream>
#include <fcntl.h>
#include "lyekka.h"
#include "cmd_handler.h"
#include "archive.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;
namespace bpo = boost::program_options;
namespace fs = boost::filesystem;

static int get_archive(CommandLineParser& c)
{
  string archive;
  bool verbose = false;
  vector<string> objects;
  c.o.add_options()
    ("archive,a", bpo::value<string>(&archive), "archive")
    ("object-sha", bpo::value< vector<string> >(&objects), "object sha");
  c.p.add("object-sha", -1);
  c.parse_options();
  
  fs::path p(archive);
  ArchiveReader ar(p);
  for (vector<string>::iterator it = objects.begin(); 
       it != objects.end();
       ++it) {
    Sha sha;
    sha.set_base16(it->c_str());
    boost::shared_ptr<ObjectInputStream> ois_p = ar.find(sha);
    FileOutputStream fos(xopen(it->c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644));
    fos.SetCloseOnDelete(true);
    copy_streams(&fos, ois_p.get());
    cout << *it << " [DONE]" << endl;
  }
}


static int ls_archive(CommandLineParser& c)
{
  string input;
  bool verbose = false;
  c.o.add_options()
    ("input,i", bpo::value<string>(&input), "input");
  c.p.add("input", 1);
  c.parse_options();
  
  fs::path p(input);
  ArchiveReader ar(p);
  cout << "Entry point: " << ar.entry_point() << endl;
  cout << "Objects:" << endl;
  for (int i = 0; i < ar.entry_count(); i++) {
    boost::shared_ptr<ArchiveObjectInputStream> ao_p = ar.find_by_idx(i);
    cout << i << ": " << ao_p->sha() << endl;
  }
  return 0;
}

LYEKKA_COMMAND(ls_archive, 
	       "ls-archive", "", 
	       "Lists contents of an archive");

LYEKKA_COMMAND(get_archive, 
	       "get-archive", "", 
	       "Retrieves an object from an archive");
