#ifndef STATUS_CMD_H_INCLUSION_GUARD
#define STATUS_CMD_H_INCLUSION_GUARD
#include "cmd_handler.h"

namespace Lyekka 
{
  
class StatusCmdHandler : public CmdHandler 
{
 public:
  const std::string get_cmd(void) { return "status"; }
  const std::string get_description(void)  { return "Shows the status of the vault."; }
  int execute(int argc, char* argv[]);
};

}




#endif
