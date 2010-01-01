#include <iostream>
#include "cmd_handler.h"
#include "paths.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;

LYEKKA_COMMAND(path, "path", "", "Manages indexed paths")
{
  return print_usage();
}

LYEKKA_COMMAND(path_add, "path/add", "<name>", "Adds path to as indexed path")
{
  string path;
  po.add_options()
    ("path", bpo::value<string>(&path), "path");
  p.add("path", 1);
  parse_options();
  
  Paths::add(path);
  cout << "Path '" << path << "' added." << endl;
  return 0;
}

LYEKKA_COMMAND(path_list, "path/list", "", "Lists indexed paths")
{
  parse_options();
  list<PathInfo> paths = Paths::get();

  for (list<PathInfo>::iterator i = paths.begin(); i != paths.end(); ++i)
    cout << i->path << endl;
  return 0;
}



