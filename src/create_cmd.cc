#include <iostream>
#include "sdsqlite/sdsqlite.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "cmd_handler.h"

using namespace std;
using namespace Lyekka;
namespace fs = boost::filesystem;

int create(CommandLineParser& c)
{
  c.parse_options();
  if (fs::exists(fs::path("lyekka.db")))
  {
    cerr << "Refuses to overwrite already existing 'lyekka.db'" << endl;
    return -2;
  }

  sd::sqlite db("lyekka.db");
  db << "BEGIN EXCLUSIVE";
  db << "CREATE TABLE paths ("  
    "id INTEGER PRIMARY KEY, " 
    "parent INTEGER, "
    "name TEXT, "
    "mtime INTEGER, "
    "chunk_id INTEGER)";

  db << "CREATE TABLE files (" 
    "id INTEGER PRIMARY KEY, "
    "parent INTEGER, "
    "name TEXT, "
    "mtime INTEGER, "
    "size INTEGER)";

  db << "CREATE TABLE objects ("
    "id INTEGER PRIMARY KEY, "
    "size INTEGER, "
    "sha BLOB COLLATE BINARY UNIQUE, "
    "key BLOB)";

  // The 'size' is associated with the object and not the part (as
  // you might think would be better) because we track remote objects
  // and they are not in the parts list, but we still need to be able
  // to sum up their sizes.
  db << "CREATE TABLE parts("
    "file_id INTEGER NOT NULL, "
    "object_id INTEGER NOT NULL, "
    "offset BIGINT)";

  db << "CREATE INDEX parts_file_id ON parts(object_id)";
  db << "CREATE INDEX parts_object_id ON parts(object_id)";

  db << "CREATE TABLE remotes("
    "id INTEGER PRIMARY KEY, "
    "name TEXT NOT NULL, "
    "default_destination TEXT NOT NULL)";
  
  db << "CREATE TABLE remote_mapping("
    "remote_id INTEGER, "
    "object_id INTEGER)";

  db << "CREATE INDEX remote_mapping_object_id ON remote_mapping(object_id)";

  db << "PRAGMA user_version = 1";
  db << "COMMIT";

  cout << "Database 'lyekka.db' created." << endl;
  return 0;
}

LYEKKA_COMMAND(create, "create", "", "Creates a new configuration");
