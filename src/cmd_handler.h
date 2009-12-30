#ifndef CMD_HANDLER_H_INCLUSION_GUARD
#define CMD_HANDLER_H_INCLUSION_GUARD
#include <string>
#include <map>
#include <exception>

namespace Lyekka 
{
  class CmdHandler
  {
  public:
    virtual const std::string get_cmd(void) = 0;
    virtual const std::string get_description(void) = 0;
    virtual int execute(int argc, char* argv[]) = 0;
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

  class CmdUsageStream {
  public:
    CmdUsageStream(void) : first(true) {};
    CmdUsageStream& operator<<(const char* str);
  private:
    bool first;
  };

class CmdUsageException : public std::exception
{
public:
  CmdUsageException(std::string error = "") : std::exception(), m_error(error) {};
  ~CmdUsageException() throw () {};
  virtual void print_usage(CmdUsageStream& os) = 0;
  void print_error(void);
private:  
  std::string m_error;
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
