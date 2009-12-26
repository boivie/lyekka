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
  db << "BEGIN EXCLUSIVE";
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
    "id INTEGER PRIMARY KEY, "
    "size INTEGER NOT NULL, "
    "sha BLOB COLLATE BINARY UNIQUE, "
    "key BLOB COLLATE BINARY)";

  db << "CREATE TABLE local_mapping("
    "file_id INTEGER NOT NULL, "
    "chunk_id INTEGER NOT NULL, "
    "offset INTEGER)";

  db << "CREATE INDEX local_mapping_file_id ON local_mapping(file_id)";
  db << "CREATE INDEX local_mapping_chunk_id ON local_mapping(chunk_id)";

  db << "CREATE TABLE remotes("
    "id INTEGER PRIMARY KEY, "
    "name TEXT NOT NULL, "
    "default_location TEXT NOT NULL)";
  
  db << "CREATE TABLE remote_mapping("
    "remote_id INTEGER, "
    "chunk_id INTEGER)";

  db << "CREATE INDEX remote_mapping_chunk_id ON remote_mapping(chunk_id)";

  db << "PRAGMA user_version = 1";
  db << "COMMIT";

  cout << "Database 'lyekka.db' created." << endl;
  return 0;
}



