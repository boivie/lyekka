#include <iostream>
#include <string>
#include "cmd_handler.h"
#include "db.h"

using namespace std;
using namespace Lyekka;

class GcCmdUsageException : public CmdUsageException
{ 
public:
  GcCmdUsageException(std::string e = "") : CmdUsageException(e) {}
  ~GcCmdUsageException() throw () {};
  void print_usage(CmdUsageStream& os) 
  {
    os << "gc";
  }
};

DECLARE_COMMAND(gc, "gc", "Cleanup and optimizes the database.");

int CMD_gc::execute(int argc, char* argv[])
{
  sd::sqlite& db = Db::get();
  
  db << "BEGIN EXCLUSIVE";
  db << "DELETE FROM chunks WHERE id NOT IN (SELECT chunk_id FROM local_mapping UNION SELECT chunk_id FROM remote_mapping)";
  db << "COMMIT"; 
  db << "VACUUM";

  return 0;
}



