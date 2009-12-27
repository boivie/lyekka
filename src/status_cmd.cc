#include <iostream>
#include "status_cmd.h"
#include "sdsqlite/sdsqlite.h"
#include "db.h"
#include "formatter.h"
#include "remotes.h"

using namespace std;
using namespace Lyekka;

int StatusCmdHandler::execute(int argc, char* argv[])
{
  sd::sqlite& db = Db::get();
  list<RemoteInfo> remotes = Remotes::get();
  for (list<RemoteInfo>::iterator i = remotes.begin(); i != remotes.end(); ++i)
  {
    sd::sql sizeq(db);
    sizeq << "SELECT SUM(size) FROM chunks WHERE id NOT IN (SELECT chunk_id FROM remote_mapping WHERE remote_id = ?)";
    sizeq << i->id;
    sizeq.step();
    
    uint64_t size;
    sizeq >> size;
    
    cout << "[" << i->name << "] At most " << Formatter::format_size(size) << " bytes needs to be copied." << endl;
  }  
  cout << endl;
  return 0;
}

