#include <iostream>
#include "cmd_handler.h"
#include "sdsqlite/sdsqlite.h"

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


CmdUsageStream& CmdUsageStream::operator<<(const char* str)
{
  if (first)
    cerr << "usage: lyekka " << str << endl;
  else
    cerr << "   or: lyekka " << str << endl;
  first = false;
  return *this;
}

void CmdUsageException::print_error(void) 
{
  if (m_error != "")
    cerr << "error: " << m_error << endl;
}

void CmdManager::print_cmdlist(void)
{
  for (map<string,CmdHandler*>::iterator i = m_handlers_p->begin(); i != m_handlers_p->end(); i++)
  {
    cout << i->second->get_cmd() << string(15 - i->second->get_cmd().size(), ' ') << i->second->get_description() << endl;
  }
}

int CmdManager::execute(string cmd, int argc, char* argv[])
{
  if (m_handlers_p->find(cmd) == m_handlers_p->end()) 
  {
    cerr << "lyekka: '" << cmd << "' is not a valid command." << endl; 
    return 1;  
  }
  try {
    return (*m_handlers_p)[cmd]->execute(argc, argv);
  } 
  catch (CmdUsageException& e)
  { 
    e.print_error();
    CmdUsageStream os;
    e.print_usage(os);
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
