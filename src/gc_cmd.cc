#include <iostream>
#include <string>
#include "cmd_handler.h"
#include "db.h"

using namespace std;
using namespace Lyekka;

static int gc(CommandLineParser& c)
{
  c.parse_options();
  sd::sqlite& db = Db::get();
  
  db << "BEGIN EXCLUSIVE";
  db << "DELETE FROM objects WHERE id NOT IN (" 
    "SELECT object_id FROM parts " 
    "UNION SELECT object_id FROM remote_mapping " 
    "UNION SELECT object_id FROM paths)";
  db << "COMMIT";
  db << "VACUUM";
  cout << "The database has been cleaned." << endl;

  return 0;
}

LYEKKA_COMMAND(gc, "gc", "", "Cleanup and optimizes the database.");

