#include "remotes.h"
#include "db.h"

using namespace Lyekka;
using namespace std;

std::list<RemoteInfo> Remotes::get(void)
{
  std::list<RemoteInfo> result;
  sd::sqlite& db = Db::get();
  sd::sql query(db);
  query << "SELECT id, name, default_destination FROM remotes ORDER BY name";
  while (query.step()) 
  { 
    RemoteInfo pi;
    query >> pi.id >> pi.name >> pi.default_destination;
    result.push_back(pi);
  }
  return result;
}

RemoteInfo Remotes::get(string& name)
{
  RemoteInfo info;
  sd::sqlite& db = Db::get();
  sd::sql query(db);
  query << "SELECT id, name, default_destination FROM remotes WHERE name = ?";
  query << name;
  if (!query.step()) 
    throw NoSuchRemoteException();

  query >> info.id >> info.name >> info.default_destination;
  return info;
}


int Remotes::add(std::string name, std::string default_destination)
{
  sd::sqlite& db = Db::get();
  sd::sql insert_query(db);
  insert_query << "INSERT INTO remotes (name, default_destination) VALUES (?, ?)";
  try 
  {
    insert_query << name << default_destination;
    insert_query.step();
  }
  catch(sd::db_error& err)
  { 
    throw RemoteAlreadyAddedException();
  }    
  
  int id = db.last_rowid();
  return id;
}

void Remotes::remove(std::string name)
{
}
  
