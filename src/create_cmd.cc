#include <iostream>
#include "sdsqlite/sdsqlite.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "create_cmd.h"

using namespace std;
using namespace Lyekka;
namespace fs = boost::filesystem;

int CreateCmdHandler::execute(int argc, char* argv[])
{
  if (fs::exists(fs::path("lyekka.db")))
  {
    cerr << "Refuses to overwrite already existing 'lyekka.db'" << endl;
    return -2;
  }

  sd::sqlite db("lyekka.db");
 
  db << "CREATE TABLE paths (id INTEGER PRIMARY KEY, path TEXT UNIQUE)";

  db << "CREATE TABLE files (" 
    "id INTEGER PRIMARY KEY, "
    "parent INTEGER, "
    "name TEXT, "
    "mtime INTEGER, "
    "ctime INTEGER, "
    "size INTEGER, "
    "type INTEGER)";  

  db << "CREATE TABLE chunks ("
    "file_id INTEGER, "
    "offset INTEGER, "
    "size INTEGER, "
    "cid BLOB COLLATE BINARY, "
    "key BLOB COLLATE BINARY)";

  db << "CREATE INDEX chunks_file_id ON chunks(file_id)";

  db << "PRAGMA user_version = 1";

  cout << "Database 'lyekka.db' created." << endl;
  return 0;
}



