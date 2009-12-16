

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/program_options.hpp>

#include <list>
#include <iostream>
#include <vector>
#include <string>
#include "main.h"
#include "index_cmd.h"
#include "create_cmd.h"
#include "path_cmd.h"

using namespace std;
using namespace Lyekka;

void Main::add_handler(CmdHandler* handler_p)
{
  handlers[handler_p->get_cmd()] = handler_p;
}

int Main::execute(int argc, char* argv[]) 
{
  CmdHandler* cmd_p = handlers[argv[1]];
  
  return cmd_p->execute(argc, argv);
}


int main(int argc, char*argv[])
{
  if (argc < 2) 
  {
    cout << "syntax: TBD" << endl;
    return -1;
  }
  else
  {
    Main me;
    
    me.add_handler(new IndexCmdHandler());
    me.add_handler(new CreateCmdHandler()); 
    me.add_handler(new PathCmdHandler());
    
    return me.execute(argc, argv);
  }
}
