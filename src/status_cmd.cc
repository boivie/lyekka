#include <iostream>
#include "status_cmd.h"
#include "sdsqlite/sdsqlite.h"
#include "db.h"
#include "formatter.h"

using namespace std;
using namespace Lyekka;

int StatusCmdHandler::execute(int argc, char* argv[])
{
  sd::sqlite& db = Db::get();
  sd::sql remotes(db);
  remotes << "SELECT id, name FROM remotes ORDER BY name";
  while (remotes.step()) {
    string name;
    int id;
    remotes >> id >> name;
    
    sd::sql sizeq(db);
    sizeq << "SELECT SUM(size) FROM chunks WHERE id NOT IN (SELECT chunk_id FROM remote_mapping WHERE remote_id = ?)";
    sizeq << id;
    sizeq.step();
    
    uint64_t size;
    sizeq >> size;
    
    cout << "[" << name << "] At most " << Formatter::format_size(size) << " bytes needs to be copied." << endl;
  }  
  cout << endl;
  return 0;
}

