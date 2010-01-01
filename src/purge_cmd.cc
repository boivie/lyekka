#include <iostream>
#include <string>
#include "cmd_handler.h"
#include "cmd_parser.h"
#include "db.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;

DECLARE_COMMAND(purge, "purge", "Purges parts of the database");

int CMD_purge::execute(int argc, char* argv[])
{
  CmdParser parser(argc, argv);
  parser.parse_command(parser.create_commands()
    ("local-index", "--force")
    ("remote-index", "--force"));

  bool force = false;
  o.add_options()("force", bpo::bool_switch(&force), "force");
  parser.parse_options(o, po, p);
  
  if (!force)
    throw parser.syntax_exception("will not do anything unless --force is specified.");

  if (parser.is_cmd("local-index"))
  {
    sd::sqlite& db = Db::get();
    db << "BEGIN EXCLUSIVE";
    db << "DELETE FROM files";
    db << "DELETE FROM local_mapping";
    db << "COMMIT";
    cout << "Local index has been purged." << endl;
  }
  else if (parser.is_cmd("remote-index"))
  {
    cerr << "Currently not supported." << endl;
    return 1;
  }

  return 0;
}



