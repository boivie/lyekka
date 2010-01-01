#include <iostream>
#include "cmd_handler.h"
#include "paths.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;

static int path_add(CommandLineParser& c)
{
  string path;
  c.po.add_options()
    ("path", bpo::value<string>(&path), "path");
  c.p.add("path", 1);
  c.parse_options();
  
  Paths::add(path);
  cout << "Path '" << path << "' added." << endl;
  return 0;
}

static int path_list(CommandLineParser& c)
{
  c.parse_options();
  list<PathInfo> paths = Paths::get();

  for (list<PathInfo>::iterator i = paths.begin(); i != paths.end(); ++i)
    cout << i->path << endl;
  return 0;
}

LYEKKA_COMMAND(path_add, "path/add", "<name>", "Adds path to as indexed path");
LYEKKA_COMMAND(path_list, "path/list", "", "Lists indexed paths");
LYEKKA_COMMAND(path_list, "path", "", "Manages indexed paths");
