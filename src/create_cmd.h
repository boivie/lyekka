#ifndef CREATE_CMD_H_INCLUSION_GUARD
#define CREATE_CMD_H_INCLUSION_GUARD
#include "cmd_handler.h"

namespace Lyekka 
{
  
class CreateCmdHandler : public CmdHandler 
{
 public:
  const std::string get_cmd(void) { return "create"; }
  const std::string get_description(void)  { return "Creates a new configuration"; }
  int execute(int argc, char* argv[]);
};

}




#endif
