#include <iostream>
#include "cmd_handler.h"
#include "remotes.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;

static int remote_list(CommandLineParser& c)
{
  bool verbose = false;
  c.o.add_options()
    ("verbose,v", bpo::bool_switch(&verbose), "be verbose");
  c.parse_options();
  list<RemoteInfo> remotes = Remotes::get();
  
  for (list<RemoteInfo>::iterator i = remotes.begin(); i != remotes.end(); ++i)
  {
    if (verbose)
      cout << i->id << ": " << i->name << " " <<  i->default_destination << endl;
    else
      cout << i->name << endl;
  }
  return 0;
}

static int remote_add(CommandLineParser& c)
{ 
  string remote_name;
  string default_destination;
  c.po.add_options()
    ("remote_name", bpo::value<string>(&remote_name), "remote name")
    ("default_destination", bpo::value<string>(&default_destination), "default destination");
  c.p.add("remote_name", 1);
  c.p.add("default_destination", 1);
  c.parse_options();

  if (remote_name == "")
    throw CommandUsageException("Missing <name>");
  
  try {
    Remotes::add(remote_name, default_destination);
  }
  catch (RemoteAlreadyAddedException& ex)
  {
    throw CommandUsageException("The path has already been added");
  }

  cout << "Remote '" << remote_name << "' added." << endl;
  return 0; 
}

LYEKKA_COMMAND(remote_list, "remote/list", "", "Lists remote sites");
LYEKKA_COMMAND(remote_list, "remote", "", "Manages remote sites");
LYEKKA_COMMAND(remote_add, "remote/add", "<name> [<default-destination>]", "Adds a remote site");

