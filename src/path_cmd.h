#ifndef PATH_CMD_H_INCLUSION_GUARD
#define PATH_CMD_H_INCLUSION_GUARD
#include "cmd_handler.h"

namespace Lyekka 
{
  
class PathCmdHandler : public CmdHandler 
{
 public:
  const std::string get_cmd(void) { return "path"; }
  const std::string get_description(void) 
  {
    return "Modifies list of indexed paths";
  }
  
  int execute(int argc, char* argv[]);
};

}




#endif
