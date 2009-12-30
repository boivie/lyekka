#include <iostream>
#include "cmd_handler.h"
#include "sdsqlite/sdsqlite.h"
#include "cmd_parser.h"

using namespace std;
using namespace Lyekka;

static int file_exists(string filename)
{
  if (FILE* file = fopen(filename.c_str(), "r"))
  {
    fclose(file);
    return true;
  }
  return false;
}

void CmdManager::print_cmdlist(void)
{
  for (map<string,CmdHandler*>::iterator i = m_handlers_p->begin(); i != m_handlers_p->end(); i++)
  {
    cout << "   " << i->second->get_cmd() << string(21 - i->second->get_cmd().size(), ' ') << i->second->get_description() << endl;
  }
}

bool CmdManager::is_command(string& cmd)
{
  return (m_handlers_p->find(cmd) != m_handlers_p->end());
}

int CmdManager::execute(string cmd, int argc, char* argv[])
{
  if (!is_command(cmd)) 
  {
    cerr << "lyekka: '" << cmd << "' is not a valid command." << endl; 
    return 1;  
  }
  try {
    return (*m_handlers_p)[cmd]->execute(argc, argv);
  } 
  catch (CmdParseException& e)
  {
    e.print_usage();
    return 129;
  }
}

void CmdManager::reg_handler(CmdHandler* handler_p)
{
  if (m_handlers_p == NULL)
    m_handlers_p = new std::map<std::string, CmdHandler*>();
  
  (*m_handlers_p)[handler_p->get_cmd()] = handler_p; 
}

std::map<std::string, CmdHandler*>* CmdManager::m_handlers_p = NULL;
