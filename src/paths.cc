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
  query << "SELECT id, name FROM paths WHERE parent = 0 ORDER BY id";
  while (query.step()) 
  { 
    PathInfo pi;
    query >> pi.id >> pi.path;
    result.push_back(pi);
  }
  return result;
}

static bool already_in_db(sd::sqlite& db, string name)
{
  sd::sql query(db);
  query << "SELECT COUNT(*) FROM paths WHERE name = ?";
  query << name; 
  query.step();
  int count;
  query >> count;
  return count > 0;
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

  if (already_in_db(db, p.string()))
    throw PathAlreadyAddedException();

  insert_query << "INSERT INTO paths (parent, name, mtime, ctime) VALUES (0, ?, 0, 0)";
  insert_query << p.string();
  insert_query.step();
  
  int id = db.last_rowid();
  return id;
}

void Paths::remove(int id)
{
}
  
