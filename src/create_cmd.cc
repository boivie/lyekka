#include <iostream>
#include "sdsqlite/sdsqlite.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "cmd_handler.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;
namespace fs = boost::filesystem;

int create(CommandLineParser& c)
{
  string name = "";
  c.po.add_options()
    ("name,n", bpo::value<string>(&name), "host's name");
  c.parse_options();

  if (fs::exists(fs::path("lyekka.db")))
  {
    cerr << "Refuses to overwrite already existing 'lyekka.db'" << endl;
    return -2;
  }

  if (name == "") {
    char buf[1024];
    if (gethostname(buf, sizeof(buf)-1) == 0) {
      name = string(buf);
    }
  }

  if (name == "") {
    cerr << "No name provided, and could not guess.";
    return 1;
  }

  sd::sqlite db("lyekka.db");
  db << "BEGIN EXCLUSIVE";
  db << "CREATE TABLE paths ("  
    "id INTEGER PRIMARY KEY, " 
    "parent INTEGER, "
    "name TEXT, "
    "mtime INTEGER, "
    "object_id INTEGER)";

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

  db << "CREATE TABLE config (name TEXT PRIMARY KEY, value TEXT)";
  db << "PRAGMA user_version = 1";

  sd::sql insert_query(db);
  insert_query << "INSERT INTO config (name, value) VALUES ('name', ?)";
  try 
  {
    insert_query << name;
    insert_query.step();
  }
  catch(sd::db_error& err)
  { 
    cerr << "Failed to set name (bad name?)" << endl;
    return 1;
  }    
  db << "COMMIT";
      
  cout << "Database 'lyekka.db' created." << endl;
  return 0;
}

LYEKKA_COMMAND(create, "create", "", "Creates a new configuration");
