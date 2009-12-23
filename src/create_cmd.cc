#include <iostream>
#include <sqlite3.h>
#include "create_cmd.h"

using namespace std;
using namespace Lyekka;

int file_exists(string filename)
{
  if (FILE* file = fopen(filename.c_str(), "r"))
  {
    fclose(file);
    return true;
  }
  return false;
}


int CreateCmdHandler::execute(int argc, char* argv[])
{
  if (file_exists("lyekka.db")) 
  {
    cerr << "Refuses to overwrite already existing 'lyekka.db'" << endl;
    return -2;
  }
  
  sqlite3* db;
  if (sqlite3_open("lyekka.db", &db) != SQLITE_OK) 
  {
    cerr << "Failed to open 'lyekka.db' for writing." << endl;
    return -2;
  }

  char* errmsg;
  if (sqlite3_exec(db, "CREATE TABLE paths (id INTEGER PRIMARY KEY, path TEXT UNIQUE)", NULL, NULL, &errmsg) != SQLITE_OK)
  {
    cerr << "DB Error: " << errmsg << endl;
    sqlite3_free(errmsg);
    return -2;
  }
  if (sqlite3_exec(db, "CREATE TABLE files (" 
      "id INTEGER PRIMARY KEY, "
      "parent INTEGER, "
      "name TEXT, "
      "mtime INTEGER, "
      "ctime INTEGER, "
      "size INTEGER, "
      "type INTEGER)", NULL, NULL, &errmsg) != SQLITE_OK)
  {
    cerr << "DB Error: " << errmsg << endl;
    sqlite3_free(errmsg);
    return -2;
  }
  if (sqlite3_exec(db, "CREATE TABLE chunks ("
      "file_id INTEGER, "
      "offset INTEGER, "
      "size INTEGER, "
      "cid BLOB COLLATE BINARY, "
		  "key BLOB COLLATE BINARY)", NULL, NULL, &errmsg) != SQLITE_OK)
  {
    cerr << "DB Error: " << errmsg << endl;
    sqlite3_free(errmsg);
    return -2;
  }
  sqlite3_exec(db, "CREATE INDEX chunks_file_id ON chunks(file_id)", NULL, NULL, &errmsg);
  if (sqlite3_exec(db, "PRAGMA user_version = 1", NULL, NULL, &errmsg) != SQLITE_OK)
  {
    cerr << "DB Error: " << errmsg << endl;
    sqlite3_free(errmsg);
    return -2;
  }
  cout << "Database created." << endl;
  return 0;
}



