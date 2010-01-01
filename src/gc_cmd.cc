#include <iostream>
#include <string>
#include "cmd_handler.h"
#include "cmd_parser.h"
#include "db.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;

DECLARE_COMMAND(gc, "gc", "Cleanup and optimizes the database.");

int CMD_gc::execute(int argc, char* argv[])
{
  CmdParser parser(argc, argv);
  parser.parse_command(parser.create_commands());

  sd::sqlite& db = Db::get();
  
  db << "BEGIN EXCLUSIVE";
  db << "DELETE FROM chunks WHERE id NOT IN (SELECT chunk_id FROM local_mapping UNION SELECT chunk_id FROM remote_mapping)";
  db << "COMMIT";
  db << "VACUUM";
  cout << "The database has been cleaned." << endl;

  return 0;
}



