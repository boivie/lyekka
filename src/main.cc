#include <list>
#include <iostream>
#include <vector>
#include <string>
#include "main.h"
#include "cmd_handler.h"
#include <boost/program_options.hpp>

using namespace std;
using namespace Lyekka;
namespace po = boost::program_options;

int Main::execute(int argc, char* argv[]) 
{
}

void print_syntax(po::options_description& desc)
{
  cout << "Syntax: lyekka [--help] [--version] CMD [ARGS]" << endl  
       << endl
       << "Available commands:" << endl;
  CmdManager::print_cmdlist();
  cout << endl;
  cout << desc << "\n";
}


int main(int argc, char*argv[])
{ 
  Main me;
  
  // Declare the supported options.
  po::options_description desc("");
  desc.add_options() 
    ("version", "print version information")
    ("help",    "prints help")
    ;

  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
  po::store(parsed, vm);
  po::notify(vm);    

  if (vm.count("version")) {
    cout << PACKAGE << " version " << PACKAGE_VERSION << endl;
    return 0;
  }

  if (argc >= 2)
  {
    string cmd(argv[1]);
    if (CmdManager::is_command(cmd))
      return CmdManager::execute(cmd, argc, argv);
    else if ((cmd != "help") && (cmd != "--help"))
    { 
      cout << "lyekka: '" << cmd << "' is not a valid command. See 'lyekka --help'" << endl;
      return 1;   
    }
  }

  print_syntax(desc);
  return 1;
}
