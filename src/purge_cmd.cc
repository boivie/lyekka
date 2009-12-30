#include <iostream>
#include <string>
#include "cmd_handler.h"
#include <boost/program_options.hpp>
#include "db.h"

using namespace std;
using namespace Lyekka;

class PurgeCmdUsageException : public CmdUsageException
{ 
public:
  PurgeCmdUsageException(std::string e = "") : CmdUsageException(e) {}
  ~PurgeCmdUsageException() throw () {};
  void print_usage(CmdUsageStream& os) 
  {
    os << "purge local-index"
       << "purge remote-index <remote-name>";
  }
};

DECLARE_COMMAND(purge, "purge", "Purges parts of the database");

int CMD_purge::execute(int argc, char* argv[])
{
  if (argc < 2)
    throw PurgeCmdUsageException();

  string cmd(argv[1]);
  if (cmd == "local-index")
  {
    sd::sqlite& db = Db::get();
    db << "BEGIN EXCLUSIVE";
    db << "DELETE FROM files";
    db << "DELETE FROM local_mapping";
    db << "COMMIT";
    cout << "Local index has been purged." << endl;
  }
  else if (cmd == "remote-index")
  {
    cerr << "Currently not supported." << endl;
    return 1;
  }
  else
  {
    throw PurgeCmdUsageException("Unknown command: " + cmd);
  }
  return 0;
}



