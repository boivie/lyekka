#include <iostream>
#include "cmd_handler.h"
#include "sdsqlite/sdsqlite.h"
#include "cmd_parser.h"

using namespace std;
using namespace Lyekka;

void CmdManager::print_cmdlist(void)
{
  for (map<string,CmdHandler*>::iterator i = m_handlers_p->begin(); i != m_handlers_p->end(); i++)
  {
    string cmd = i->second->get_cmd();
    // Don't print sub commands
    if (cmd.find('/') == string::npos)
    {
      cout << "   " << i->second->get_cmd() << string(21 - i->second->get_cmd().size(), ' ') << i->second->get_description() << endl;
    }
  }
}

int CmdManager::execute(int argc, char* argv[])
{
  // See if the first argument is a valid command
  string cmd(argv[1]);
  
  if (m_handlers_p->find(cmd) == m_handlers_p->end())
  {
    cerr << "lyekka: '" << cmd << "' is not a lyekka commmand. See 'lyekka --help'." << endl;
    return 129;
  }

  if (argc >= 3)
  {
    // Found a matching command. Note that we may have sub-commands - in that case, we must see if one is matching.
    string subcommand(argv[1]);
    subcommand += "/";
    for (std::map<std::string, CmdHandler*>::iterator i = m_handlers_p->begin(); i != m_handlers_p->end(); ++i)
    {
      if (i->first.compare(0, subcommand.size(), subcommand) == 0)
      { 
        // The command has subcommands. Check if the next parameter is a command (and not a switch), and in that case - invoke it.
        if (argv[2][0] != '-')
        {
          subcommand += string(argv[2]);
          if (m_handlers_p->find(subcommand) == m_handlers_p->end())
          {
            cerr << "lyekka: '" << argv[1] << " " << argv[2] << "' is not a lyekka command. See 'lyekka " << argv[1] << " --help'." << endl;
            return 129;
          }
          // Call sub-command.
          return (*m_handlers_p)[subcommand]->execute(argc - 2, argv + 2);
        } 
      }
    }
  }

  // Call command.
  return (*m_handlers_p)[cmd]->execute(argc - 1, argv + 1);
}

void CmdManager::reg_handler(CmdHandler* handler_p)
{
  if (m_handlers_p == NULL)
    m_handlers_p = new std::map<std::string, CmdHandler*>();
  
  (*m_handlers_p)[handler_p->get_cmd()] = handler_p; 
}

std::map<std::string, CmdHandler*>* CmdManager::m_handlers_p = NULL;

int LyCommand::print_usage(const char* extra)
{
  if (extra[0] != 0)
    cout << "lyekka: " << extra << endl;
  cout << "USAGE" << endl;
  return 0;
}

int LyCommand::execute(int argc, char* argv[])
{
  m_argc = argc;
  m_argv = argv;
  try
  {
    return run();
  }
  catch (boost::program_options::unknown_option& ex)
  {
    return print_usage(ex.what());
  }
}

void LyCommand::parse_options(void)
{
  boost::program_options::options_description ao;
  o.add_options()("help", "shows help");
  ao.add(o).add(po);
  boost::program_options::store(boost::program_options::command_line_parser(m_argc, m_argv).
                                options(ao).positional(p).run(), m_vm);
  boost::program_options::notify(m_vm);
  if (m_vm.count("help"))
    throw "help";
}



