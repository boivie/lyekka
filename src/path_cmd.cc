#include <iostream>
#include "path_cmd.h"
#include "paths.h"

using namespace std;
using namespace Lyekka;

void print_syntax(void)
{
  cerr << "Syntax: path [add] [remove] [list]" << endl;
}

int PathCmdHandler::execute(int argc, char* argv[])
{
  if (argc < 3)
  {
    print_syntax();
    return -2;
  }

  string cmd(argv[2]);

  try 
  { 
    if (cmd == "add")
    { 
      string path(argv[3]);
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



