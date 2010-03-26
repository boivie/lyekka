#include <iostream>
#include "sdsqlite/sdsqlite.h"
#include "db.h"
#include "formatter.h"
#include "remotes.h"
#include "cmd_handler.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;

void print_remote_status(sd::sqlite& db, RemoteInfo& ri)
{
  sd::sql sizeq(db);
  sizeq << "SELECT SUM(c.size) FROM objects c, parts l WHERE l.object_id = c.id AND c.id NOT IN (SELECT object_id FROM remote_mapping WHERE remote_id = ?)";
  sizeq << ri.id;
  sizeq.step();
    
  uint64_t size;
  sizeq >> size;
    
  // TODO: Path chunks are not included in this.

  cout << "[" << ri.name << "] At most " << Formatter::format_size(size) << " bytes needs to be copied." << endl;
}

static int status(CommandLineParser& c)
{
  string remote_name;
  c.po.add_options()
    ("remote_name", bpo::value<string>(&remote_name), "remote name");
  c.p.add("remote_name", 1);
  c.parse_options();
  
  sd::sqlite& db = Db::get();
  if (remote_name != "") 
  {
    RemoteInfo ri = Remotes::get(remote_name);
    print_remote_status(db, ri);
  }
  else
  {
    list<RemoteInfo> remotes = Remotes::get();
    for (list<RemoteInfo>::iterator i = remotes.begin(); i != remotes.end(); ++i)
    { 
      RemoteInfo& ri = *i;
      print_remote_status(db, ri);
    }  
  }
  return 0;
}


LYEKKA_COMMAND(status, "status", "[<remote-name>]", "Shows the status of the vault");

