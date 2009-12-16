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
  if (sqlite3_exec(db, "CREATE TABLE paths (id INTEGER PRIMARY KEY, path TEXT)", NULL, NULL, &errmsg) != SQLITE_OK)
  {
    cerr << "DB Error: " << errmsg << endl;
    sqlite3_free(errmsg);
    return -2;
  }
  cout << "Database created." << endl;
  return 0;
}



