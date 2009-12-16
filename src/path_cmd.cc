#include <iostream>
#include "path_cmd.h"
#include "sdsqlite/sdsqlite.h"
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;

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
    fs::path p( argv[3], fs::native );
    p = fs::system_complete(p);
    
    if ( !fs::exists( p ) )
    { 
      std::cerr << "not found: " << p << std::endl;
      return -2;
    }
    else if (!fs::is_directory(p)) 
    {
      std::cerr << p << " is not a directory." << endl;
      return -2;
    }
    
    sd::sqlite db = get_db();
    sd::sql insert_query(db);
    insert_query << "INSERT INTO paths (path) VALUES (?)";
    try 
    {
      insert_query << p.string();
      insert_query.step();
    }
    catch(sd::db_error& err)
    {
      cerr << p << " is already present." << endl;
      return -2;
    }    
    
    int id = db.last_rowid();
    cout << "Path '" << p << "' added." << endl;
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
      cout << "[" << id << "] " << path << endl;
    }
  }
  else
  {
    print_syntax();
    return -2;
  }
  return 0;
  
}



