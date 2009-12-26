#include <iostream>
#include "remote_cmd.h"
#include "remotes.h"

using namespace std;
using namespace Lyekka;

class RemoteCmdUsageException : public CmdUsageException
{
  void print_usage(void) 
  {
    cerr << "usage: lyekka remote" << endl
         << "   or: lyekka remote list" << endl
         << "   or: lyekka remote add <name> [<default destination>]" << endl
         << "   or: lyekka remote rm <name>" << endl;
  }
};

int RemoteCmdHandler::execute(int argc, char* argv[])
{
  string cmd;
  if (argc >= 3)
    cmd = argv[2];

  try 
  { 
    if (cmd == "add")
    { 
      if (argc < 4 || argc >= 6) {
        throw RemoteCmdUsageException();
      }
      string remote(argv[3]);
      string default_destination;
      if (argc == 5)
        default_destination = argv[4];
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
      throw RemoteCmdUsageException();
    }
  }
  catch (RemoteException& e)
  {
    cerr << e.what() << endl;
    return -2;
  }

  return 0;
}



