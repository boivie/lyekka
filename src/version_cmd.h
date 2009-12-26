#ifndef VERSION_CMD_H_INCLUSION_GUARD
#define VERSION_CMD_H_INCLUSION_GUARD
#include "cmd_handler.h"

namespace Lyekka 
{
  
class VersionCmdHandler : public CmdHandler 
{
 public:
  const std::string get_cmd(void) { return "version"; }
  const std::string get_description(void)  { return "Shows the version of this program."; }
  int execute(int argc, char* argv[]);
};

}




#endif
