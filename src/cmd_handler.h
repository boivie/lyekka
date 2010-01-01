#ifndef CMD_HANDLER_H_INCLUSION_GUARD
#define CMD_HANDLER_H_INCLUSION_GUARD
#include <string>
#include <map>
#include <exception>
#include <iostream>
#include <boost/program_options.hpp>

namespace Lyekka 
{
  class CommandUsageException
  {
  public:
    CommandUsageException(const std::string extra = "") : m_extra(extra) {};
    const char* what() { return m_extra.c_str(); }
  private:
    const std::string m_extra;
  };

  class CommandLineParser 
  {
  public:
    CommandLineParser(int argc, char* argv[]) : m_argc(argc), m_argv(argv) {}
    boost::program_options::options_description o, po;
    boost::program_options::positional_options_description p;
    void parse_options(void);
    boost::program_options::variables_map vm;
  private:
    int m_argc;
    char** m_argv;
  };

  class CmdHandler
  {
  public:
    virtual const std::string get_cmd(void) = 0;
    virtual const std::string get_description(void) = 0;
    virtual int execute(int argc, char* argv[]) = 0;
    virtual const std::string get_args() { ""; }

  protected:
    boost::program_options::options_description o, po;
    boost::program_options::positional_options_description p;
  };

  class CmdManager 
  {
  public:
    static void reg_handler(CmdHandler* handler_p);
    static void print_sub_cmdlist(const std::string prefix);
    static void print_main_cmdlist();
    static int execute(int argc, char* argv[]);
  private:
    static std::map<std::string, CmdHandler*>* m_handlers_p;
  };

  typedef int (*CommandHandler)(CommandLineParser&);

  class LyCommand : public CmdHandler
  {
  public:
    LyCommand(CommandHandler cmdh, const char* cmd, const char* args, const char* descr)
      : m_cmdh(cmdh), m_cmd(cmd), m_args(args), m_descr(descr) { CmdManager::reg_handler(this); }
    int execute(int argc, char* argv[]);
    const std::string get_cmd() { return m_cmd; }
    const std::string get_args() { return m_args; }
    const std::string get_description() { return m_descr; }
  protected:
    int print_usage(CommandLineParser& c, const char* extra = "");
    CommandHandler m_cmdh;
    const char* m_cmd;
    const char* m_args;
    const char* m_descr;
  };
}

#define DECLARE_COMMAND(id, cmd, description) \
class CMD_ ## id : public CmdHandler { \
public: \
 CMD_ ## id (void) { CmdManager::reg_handler(this); } \
  const std::string get_cmd() { return cmd; }; \
  const std::string get_description() { return description; } \
  int execute(int argc, char* argv[]); \
}; \
static CMD_ ## id __cmd_ ## id

#define CONCAT(a,b) a ## b

#define MAKE_ID(a, b)                             \
  CONCAT(a, b)

#define LYEKKA_COMMAND(func, cmd, args, description) \
  static LyCommand MAKE_ID(func, __LINE__)(func, cmd, args, description)

#endif
