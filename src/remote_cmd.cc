#include <iostream>
#include "cmd_handler.h"
#include "cmd_parser.h"
#include "remotes.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;

DECLARE_COMMAND(remote, "remote", "Manages remote destinations");

int CMD_remote::execute(int argc, char* argv[])
{
  CmdParser parser(argc, argv);
  parser.parse_command(parser.create_commands()
    ("list", "")
    ("add", "<name> [<default destination>]")
    ("rm", "<name>"));

  try 
  { 
    if (parser.is_cmd("add"))
    { 
      string remote_name;
      string default_destination;
      bpo::options_description o, po;
      bpo::positional_options_description p;
      po.add_options()
        ("remote_name", bpo::value<string>(&remote_name), "remote name")
        ("default_destination", bpo::value<string>(&default_destination), "default destination");
      p.add("remote_name", 1);
      p.add("default_destination", 1);

      parser.parse_options(o, po, p);
      if (remote_name == "")
        throw parser.syntax_exception("Missing <name>");

      Remotes::add(remote_name, default_destination);
      cout << "Remote '" << remote_name << "' added." << endl;
    }
    else if (parser.is_cmd("list") || parser.is_cmd(""))
    {
      bool verbose = false;
      bpo::options_description o, po;
      bpo::positional_options_description p;
      o.add_options()
        ("verbose,v", bpo::bool_switch(&verbose), "be verbose");
      parser.parse_options(o, po, p);
      list<RemoteInfo> remotes = Remotes::get();

      for (list<RemoteInfo>::iterator i = remotes.begin(); i != remotes.end(); ++i)
      {
        if (verbose)
          cout << i->id << ": " << i->name << " " <<  i->default_destination << endl;
        else
          cout << i->name << endl;
      }
    }
  }
  catch (RemoteException& e)
  {
    cerr << e.what() << endl;
    return -2;
  }

  return 0;
}



