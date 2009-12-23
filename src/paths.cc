#include "paths.h"
#include "db.h"
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;

using namespace Lyekka;
using namespace std;


std::list<PathInfo> Paths::get(void)
{
  std::list<PathInfo> result;
  sd::sqlite& db = Db::get();
  sd::sql query(db);
  query << "SELECT id, path FROM paths ORDER BY id";
  while (query.step()) 
  { 
    PathInfo pi;
    query >> pi.id >> pi.path;
    result.push_back(pi);
  }
  return result;
}

int Paths::add(std::string& path)
{
  fs::path p(path, fs::native);
  p = fs::system_complete(p);
  
  if (!fs::exists(p) || 
    !fs::is_directory(p))
  { 
    throw InvalidPathException();
  }
  
  sd::sqlite& db = Db::get();
  sd::sql insert_query(db);
  insert_query << "INSERT INTO paths (path) VALUES (?)";
  try 
  {
    insert_query << p.string();
    insert_query.step();
  }
  catch(sd::db_error& err)
  { 
    throw PathAlreadyAddedException();
  }    
  
  int id = db.last_rowid();
  return id;
}

void Paths::remove(int id)
{
}
  
