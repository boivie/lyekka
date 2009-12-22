#ifndef INDEX_CMD_H_INCLUSION_GUARD
#define INDEX_CMD_H_INCLUSION_GUARD
#include "cmd_handler.h"

namespace Lyekka 
{
  
class IndexCmdHandler : public CmdHandler 
{
 public:
  const std::string get_cmd(void) { return "update-index"; }
  const std::string get_description(void) 
  {
    return "Updates the index";
  }
  
  int execute(int argc, char* argv[]);
};

}




#endif
