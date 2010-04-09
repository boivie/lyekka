#include <boost/filesystem.hpp>
#include <iomanip>
#include <iostream>
#include "lyekka.h"
#include "cmd_handler.h"
#include "archive.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;
namespace fs = boost::filesystem;

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
