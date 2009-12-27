#include <iostream>
#include "cmd_handler.h"
#include "paths.h"

using namespace std;
using namespace Lyekka;

class PathCmdUsageException : public CmdUsageException
{ 
public:
  PathCmdUsageException(std::string e = "") : CmdUsageException(e) {}
  ~PathCmdUsageException() throw () {};
  void print_usage(CmdUsageStream& os) 
  {
    os << "path" 
       << "path list" 
       << "path add <path>"
       << "path rm <path>";
  }
};

DECLARE_COMMAND(path, "path", "Manages indexed paths");

int CMD_path::execute(int argc, char* argv[])
{
  string cmd;
  if (argc > 1)
    cmd = string(argv[1]);

  try 
  { 
    if (cmd == "add")
    { 
      string path(argv[2]);
      Paths::add(path);
      cout << "Path '" << path << "' added." << endl;
    }
    else if (cmd == "" || cmd == "list") 
    {
      list<PathInfo> paths = Paths::get();
      for (list<PathInfo>::iterator i = paths.begin(); i != paths.end(); ++i)
        cout << i->path << endl;
    }
    else
    {
      throw PathCmdUsageException("unknown command: '" + cmd + "'");
    }
  }
  catch (PathException& e)
  {
    cerr << e.what() << endl;
    return 1;
  }

  return 0;
}



