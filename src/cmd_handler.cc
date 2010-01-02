#include <iostream>
#include "cmd_handler.h"
#include "sdsqlite/sdsqlite.h"

using namespace std;
using namespace Lyekka;

void CmdManager::print_main_cmdlist()
{
  for (map<string,Command*>::iterator i = m_handlers_p->begin(); i != m_handlers_p->end(); i++)
  {
    string cmd = i->second->get_cmd();

    if (cmd.find('/') == string::npos)
    {
      cout << "   " << cmd << string(21 - i->second->get_cmd().size(), ' ') << i->second->get_description() << endl;
    }
  }
}

string& string_snr(string& str, const string& search, const string& replace)
{
  string::size_type pos = 0;
  while ((pos = str.find(search, pos)) != string::npos) {
    str.replace(pos, search.size(), replace);
    pos++;
  }
  return str;
}

void CmdManager::print_sub_cmdlist(const string prefix)
{
  bool first = true;
  for (map<string,Command*>::iterator i = m_handlers_p->begin(); i != m_handlers_p->end(); i++)
  {
    string cmd = i->second->get_cmd();

    if (cmd.compare(0, prefix.size(), prefix) != 0)
      continue;

    if (first)
      cout << "usage: ";
    else  
      cout << "   or: ";
    first = false;
    cout << string_snr(cmd, "/", " ") << " " << i->second->get_args() << endl;
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
    for (std::map<std::string, Command*>::iterator i = m_handlers_p->begin(); i != m_handlers_p->end(); ++i)
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

void CmdManager::reg_handler(Command* handler_p)
{
  if (m_handlers_p == NULL)
    m_handlers_p = new std::map<std::string, Command*>();
  
  (*m_handlers_p)[handler_p->get_cmd()] = handler_p; 
}

std::map<std::string, Command*>* CmdManager::m_handlers_p = NULL;

int Command::print_usage(CommandLineParser& c, const char* extra)
{
  if (extra[0] != 0)
    cout << "error: " << extra << endl;
  
  CmdManager::print_sub_cmdlist(get_cmd());
  cout << endl;
  cout << "Available Options:" << endl;
  cout << c.o << endl;
  return 0;
}

int Command::execute(int argc, char* argv[])
{
  CommandLineParser c(argc, argv);
  try
  {
    return m_cmdh(c);
  }
  catch (boost::program_options::unknown_option& ex)
  {
    return print_usage(c, ex.what());
  }
  catch (CommandUsageException& ex)
  {
    return print_usage(c, ex.what());
  }
}

void CommandLineParser::parse_options(void)
{
  boost::program_options::options_description ao;
  o.add_options()("help", "shows help");
  ao.add(o).add(po);
  boost::program_options::store(boost::program_options::command_line_parser(m_argc, m_argv).
                                options(ao).positional(p).run(), vm);
  boost::program_options::notify(vm);
  if (vm.count("help"))
    throw CommandUsageException();
}



