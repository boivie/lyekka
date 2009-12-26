#ifndef REMOTE_CMD_H_INCLUSION_GUARD
#define REMOTE_CMD_H_INCLUSION_GUARD
#include "cmd_handler.h"

namespace Lyekka 
{
  
class RemoteCmdHandler : public CmdHandler 
{
 public:
  const std::string get_cmd(void) { return "remote"; }
  const std::string get_description(void) { return "Manages remote clients"; }
  int execute(int argc, char* argv[]);
};

}




#endif
