

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
  if (handlers.find(argv[1]) == handlers.end())
    return -1;
  
  return handlers[argv[1]]->execute(argc, argv);
}

void Main::print_syntax(void)
{
  map<string,CmdHandler*>::iterator i;
  cout << "Syntax: lyekka [CMD] [ARGS]" << endl;
  
  for (i = handlers.begin(); i != handlers.end(); i++)
  {
    cout << i->second->get_cmd() << string(15 - i->second->get_cmd().size(), ' ') << i->second->get_description() << endl;
  }
}


int main(int argc, char*argv[])
{ 
  Main me;
    
  me.add_handler(new IndexCmdHandler());
  me.add_handler(new CreateCmdHandler()); 
  me.add_handler(new PathCmdHandler());
  
  if (argc < 2) 
  {
    me.print_syntax();
    return -1;
  }
  else
  {
    
    int ret = me.execute(argc, argv);
    if (ret == -1)
      me.print_syntax();
    
    return ret;
  }
}
