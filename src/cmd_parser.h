#ifndef CMD_PARSER_H_INCLUSION_GUARD
#define CMD_PARSER_H_INCLUSION_GUARD

#include <list>
#include <string>
#include <boost/program_options.hpp>
#include "cmd_handler.h"

namespace Lyekka {
  class Command {
  public:
    Command(std::string n, std::string a) : name(n), args(a) {}
    std::string name;
    std::string args;
  };

  class Commands {
  public:
    Commands& operator()(const char* name, const char* args);
    const std::list<Command> get_list() const { return m_cmds; }
  private:
    std::list<Command> m_cmds;
  };

  class CmdParseException {
  public: 
    ~CmdParseException() throw () {};
    CmdParseException(const char* name, const std::list<Command>& list, std::string extra = "") 
      : m_name(name), m_list(list), m_extra(extra) {}
    CmdParseException(const char* name, const std::list<Command>& list, boost::program_options::options_description& desc, std::string extra) 
      : m_name(name), m_list(list), m_desc(desc), m_extra(extra) {}
    void print_usage(void);
  private:
    std::string m_name;
    std::string m_extra;
    std::list<Command> m_list;
    boost::program_options::options_description m_desc;
  };

  class CmdParser 
  {
  public: 
    CmdParser(int argc, char* argv[]) : m_argc(argc), m_argv(argv), m_has_cmd(false) {}
    bool is_cmd(std::string cmd);
    void parse_command(Commands& commands);
    Commands& create_commands() { return m_cmds; }
    void parse_options(
      boost::program_options::options_description& o,
      boost::program_options::options_description& po,
      boost::program_options::positional_options_description& p);
    boost::program_options::variables_map& vm() { return m_vm; }
    CmdParseException syntax_exception(std::string extra = "");
  private:
    bool m_has_cmd;
    int m_argc;
    char** m_argv;
    boost::program_options::variables_map m_vm;
    Commands m_cmds;
  };

}

#endif
