#include <iostream>
#include "cmd_handler.h"
#include "cmd_parser.h"
#include "paths.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;

DECLARE_COMMAND(path, "path", "Manages indexed paths");

int CMD_path::execute(int argc, char* argv[])
{
  CmdParser parser(argc, argv);
  parser.parse_command(parser.create_commands()
    ("list", "")
    ("add", "<name>")
    ("rm", "<name>"));

  try 
  { 
    if (parser.is_cmd("add"))
    { 
      string path;
      po.add_options()
        ("path", bpo::value<string>(&path), "path");
      p.add("path", 1);
      parser.parse_options(o, po, p);

      Paths::add(path);
      cout << "Path '" << path << "' added." << endl;
    }
    else if (parser.is_cmd("list"))
    {
      string path;
      parser.parse_options(o, po, p);

      list<PathInfo> paths = Paths::get();
      for (list<PathInfo>::iterator i = paths.begin(); i != paths.end(); ++i)
        cout << i->path << endl;
    }
  }
  catch (PathException& e)
  {
    throw parser.syntax_exception(e.what());
  }

  return 0;
}



