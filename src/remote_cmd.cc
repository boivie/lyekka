#include <iostream>
#include "cmd_handler.h"
#include "remotes.h"

using namespace std;
using namespace Lyekka;

class RemoteCmdUsageException : public CmdUsageException
{ 
public:
  RemoteCmdUsageException(std::string e = "") : CmdUsageException(e) {}
  ~RemoteCmdUsageException() throw () {};
  void print_usage(CmdUsageStream& os) 
  {
    os << "remote" 
       << "remote list" 
       << "remote add <name> [<default destination>]" 
       << "remote rm <name>";
  }
};

DECLARE_COMMAND(remote, "remote", "Manages remote destinations");

int CMD_remote::execute(int argc, char* argv[])
{
  string cmd;
  if (argc >= 2)
    cmd = argv[1];

  try 
  { 
    if (cmd == "add")
    { 
      if (argc < 3 || argc >= 5) {
        throw RemoteCmdUsageException();
      }
      string remote(argv[2]);
      string default_destination;
      if (argc == 4)
        default_destination = argv[3];
      Remotes::add(remote, default_destination);
      cout << "Remote '" << remote << "' added." << endl;
    }
    else if (cmd == "list" || cmd == "") 
    {
      list<RemoteInfo> remotes = Remotes::get();
      cout << "Remote sites:" << endl;
      for (list<RemoteInfo>::iterator i = remotes.begin(); i != remotes.end(); ++i)
        cout << i->name << endl;
    }
    else
    {
      throw RemoteCmdUsageException("Unknown subcommand: " + cmd);
    }
  }
  catch (RemoteException& e)
  {
    cerr << e.what() << endl;
    return -2;
  }

  return 0;
}



