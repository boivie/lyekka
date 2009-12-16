#include <iostream>
#include "path_cmd.h"
#include "sdsqlite.h"

using namespace std;
using namespace Lyekka;

void print_syntax(void)
{
  cerr << "Syntax: path [add] [remove] [list]" << endl;
}

int PathCmdHandler::execute(int argc, char* argv[])
{
  if (argc < 3)
  {
    print_syntax();
    return -2;
  }

  string cmd = string(argv[2]);
  

  if ((cmd == "add") && (argc >= 4))
  {
    sd::sqlite db = get_db();
    sd::sql insert_query(db);
    insert_query << "INSERT INTO paths (path) VALUES (?)";
    insert_query << string(argv[3]);
    insert_query.step();
    int id = db.last_rowid();
    cout << "Path " << argv[3] << " inserted as ID " << id << "." << endl;
  }
  else if (cmd == "list") 
  {
    sd::sqlite db = get_db();
    sd::sql query(db);
    query << "SELECT id, path FROM paths ORDER BY id";
    int id;
    string path;
    cout << "Indexed paths:" << endl;
    while (query.step()) 
    { 
      query >> id >> path;
      cout << id << ": " << path << endl;
    }
  }
  else
  {
    print_syntax();
    return -2;
  }
  return 0;
  
}



