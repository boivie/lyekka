#include "cmd_parser.h"
#include <iostream>

using namespace Lyekka;
using namespace std;


Commands& Commands::operator()(const char* name, const char* args)
{
  string n(name);
  string a(args);
  Command cmd(n, a);
  m_cmds.push_back(cmd);
  return *this;
}

void CmdParseException::print_usage(void)
{
  if (m_extra != "")
  {
    cerr << "error: " << m_extra << endl;
  }
  for (list<Command>::iterator i = m_list.begin(); i != m_list.end(); ++i)
  {
    if (i == m_list.begin())
      cerr << "usage:";
    else
      cerr << "   or:";
    
    cerr << " lyekka " << m_name << " " << i->name << " " << i->args << endl;
  }
  
  if (m_desc.options().size() > 0)
  {
    cerr << m_desc;
  }
}

bool CmdParser::is_cmd(std::string cmd)
{
  string arg_cmd(m_argv[2]);
  return arg_cmd == cmd;
}

void CmdParser::parse_command(Commands& commands)
{
  if (m_argc < 3)
    throw CmdParseException(m_argv[1], commands.get_list());

  m_has_cmd = true;
  string arg_cmd(m_argv[2]);
  bool found_cmd = false;
  const std::list<Command> cmds = commands.get_list();
  for (list<Command>::const_iterator i = cmds.begin(); i != cmds.end(); ++i)
  {
    if (i->name == arg_cmd)
      found_cmd = true; 
  }

  if (!found_cmd)
    throw CmdParseException(m_argv[1], commands.get_list(), "invalid command: '" + arg_cmd + "'");
}

void CmdParser::parse_options(
  boost::program_options::options_description& o,
  boost::program_options::options_description& po,
  boost::program_options::positional_options_description& p)
{
  boost::program_options::options_description ao;
  ao.add(o).add(po);
  boost::program_options::store(boost::program_options::command_line_parser(m_argc - 2, m_argv + 2).
                                options(ao).positional(p).run(), m_vm);
  boost::program_options::notify(m_vm);
}

CmdParseException CmdParser::syntax_exception(std::string extra)
{
  return CmdParseException(m_argv[1], m_cmds.get_list(), extra);
}


