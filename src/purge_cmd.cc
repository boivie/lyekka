#include <iostream>
#include <string>
#include "cmd_handler.h"
#include "db.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;

static int purge_local(CommandLineParser& c)
{
  bool force = false;
  c.o.add_options()("force", bpo::bool_switch(&force), "force");
  c.parse_options();
  
  if (!force)
    throw CommandUsageException("will not do anything unless --force is specified.");

  sd::sqlite& db = Db::get();
  db << "BEGIN EXCLUSIVE";
  db << "DELETE FROM files";
  db << "DELETE FROM local_mapping";
  db << "COMMIT";
  cout << "Local index has been purged." << endl;
  return 0;
}

static int purge_remote(CommandLineParser& c)
{
  bool force = false;
  c.o.add_options()("force", bpo::bool_switch(&force), "force");
  c.parse_options();
  
  if (!force)
    throw CommandUsageException("will not do anything unless --force is specified.");

  cout << "Currently not supported" << endl;
  return 1;
}

static int purge(CommandLineParser& c)
{
  c.parse_options();
  throw CommandUsageException("");
}




LYEKKA_COMMAND(purge, "purge", "", "Purges the index");
LYEKKA_COMMAND(purge_local, "purge/local-index", "[--force]", "Purges local index");
LYEKKA_COMMAND(purge_remote, "purge/remote-index", "[--force]", "Purges remote index");
