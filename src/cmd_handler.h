#ifndef CMD_HANDLER_H_INCLUSION_GUARD
#define CMD_HANDLER_H_INCLUSION_GUARD
#include <string>
#include <map>
#include <exception>
#include <boost/program_options.hpp>

namespace Lyekka 
{
  class CmdHandler
  {
  public:
    virtual const std::string get_cmd(void) = 0;
    virtual const std::string get_description(void) = 0;
    virtual int execute(int argc, char* argv[]) = 0;
  protected:
    boost::program_options::options_description o, po;
    boost::program_options::positional_options_description p;
  };

  class CmdManager 
  {
  public:
    static void reg_handler(CmdHandler* handler_p);
    static void print_cmdlist(void);
    static int execute(int argc, char* argv[]);
  private:
    static std::map<std::string, CmdHandler*>* m_handlers_p;
  };

  class LyCommand : public CmdHandler
  {
  public:
    int execute(int argc, char* argv[]);
    virtual int run(void) = 0;
  protected:
    int print_usage(const char* extra = "");
    void parse_options(void);
    boost::program_options::variables_map m_vm;
  private:
    int m_argc;
    char** m_argv;
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


#define LYEKKA_COMMAND(id, cmd, args, description) \
class CMD_ ## id ## _impl : public LyCommand { \
public: \
 CMD_ ## id ## _impl(void) { CmdManager::reg_handler(this); } \
  const std::string get_cmd() { return cmd; }; \
  const std::string get_args() { return args; }; \
  const std::string get_description() { return description; } \
  int run(); \
}; \
static CMD_ ## id ## _impl __cmd_ ## id; \
int CMD_ ## id ## _impl::run()

#endif
