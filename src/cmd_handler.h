#ifndef CMD_HANDLER_H_INCLUSION_GUARD
#define CMD_HANDLER_H_INCLUSION_GUARD
#include <string>
#include "sdsqlite.h"

namespace Lyekka 
{
  
class CmdHandler 
{
 public:
  virtual const std::string get_cmd(void) = 0;
  virtual const std::string get_description(void) = 0;
  virtual int execute(int argc, char* argv[]) = 0;
 protected:
  sd::sqlite get_db();
  
};

}

#endif
