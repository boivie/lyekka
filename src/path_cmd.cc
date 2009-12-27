#include <iostream>
#include "cmd_handler.h"
#include "paths.h"

using namespace std;
using namespace Lyekka;

void print_syntax(void)
{
  cerr << "Syntax: path [add] [remove] [list]" << endl;
}

DECLARE_COMMAND(path, "path", "Manages indexed paths");

int CMD_path::execute(int argc, char* argv[])
{
  if (argc < 2)
  {
    print_syntax();
    return -2;
  }

  string cmd(argv[1]);

  try 
  { 
    if (cmd == "add")
    { 
      string path(argv[2]);
      Paths::add(path);
      cout << "Path '" << path << "' added." << endl;
    }
    else if (cmd == "list") 
    {
      list<PathInfo> paths = Paths::get();
      cout << "Indexed paths:" << endl;
      for (list<PathInfo>::iterator i = paths.begin(); i != paths.end(); ++i)
        cout << "[" << i->id << "] " << i->path << endl;
    }
    else
    {
      print_syntax();
      return -2;
    }
  }
  catch (PathException& e)
  {
    cerr << e.what() << endl;
    return -2;
  }

  return 0;
}



