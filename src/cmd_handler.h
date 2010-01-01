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
    static int execute(std::string cmd, int argc, char* argv[]);
    static bool is_command(std::string& cmd);
  private:
    static std::map<std::string, CmdHandler*>* m_handlers_p;
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

#endif
