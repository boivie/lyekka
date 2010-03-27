#include <list>
#include <iostream>
#include <vector>
#include <string>
#include "main.h"
#include "cmd_handler.h"

using namespace std;
using namespace Lyekka;

void print_syntax()
{
  cout << "Syntax: lyekka [--help] [--version] CMD [ARGS]" << endl  
       << endl
       << "Available commands:" << endl;
  CmdManager::print_main_cmdlist();
}


int main(int argc, char*argv[])
{ 
  Main me;

  // A little sluggish handling of switches.
  if (argc > 1)
  {
    string arg1(argv[1]);
    if (arg1 == "--version")
    {
      cout << "lyekka version 0.1" << endl;
      return 0;
    }
    else if (arg1 != "--help")
    {
      return CmdManager::execute(argc, argv);
    }
  }

  print_syntax();
  return 1;
}
