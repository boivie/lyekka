

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/program_options.hpp>

#include <list>
#include <iostream>
#include <vector>
#include <string>
#include "main.h"
#include "cmd_handler.h"

using namespace std;
using namespace Lyekka;

int Main::execute(int argc, char* argv[]) 
{
}

void Main::print_syntax(void)
{
  cout << "Syntax: lyekka [CMD] [ARGS]" << endl;
  
  CmdManager::print_cmdlist();
}


int main(int argc, char*argv[])
{ 
  Main me;
    
  if (argc < 2) 
  {
    me.print_syntax();
    return -1;
  }
  else
  {
    string cmd(argv[1]);
    return CmdManager::execute(cmd, argc - 1, argv + 1);
  }
}
