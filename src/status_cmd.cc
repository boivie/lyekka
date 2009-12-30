#include <iostream>
#include "sdsqlite/sdsqlite.h"
#include "db.h"
#include "formatter.h"
#include "remotes.h"
#include "cmd_handler.h"
#include "cmd_parser.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;

DECLARE_COMMAND(status, "status", "Shows the status of the vault");

void print_remote_status(sd::sqlite& db, RemoteInfo& ri)
{
  sd::sql sizeq(db);
  sizeq << "SELECT SUM(c.size) FROM chunks c, local_mapping l WHERE l.chunk_id = c.id AND c.id NOT IN (SELECT chunk_id FROM remote_mapping WHERE remote_id = ?)";
  sizeq << ri.id;
  sizeq.step();
    
  uint64_t size;
  sizeq >> size;
    
  cout << "[" << ri.name << "] At most " << Formatter::format_size(size) << " bytes needs to be copied." << endl;
}

int CMD_status::execute(int argc, char* argv[])
{
  CmdParser parser(argc, argv);
  parser.parse_command(parser.create_commands()
    ("", "[<remote-name>]"));
  string remote_name;
  bpo::options_description o, po;
  bpo::positional_options_description p;
  po.add_options()
    ("remote_name", bpo::value<string>(&remote_name), "remote name");
  p.add("remote_name", 1);
  parser.parse_options(o, po, p);
  
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

