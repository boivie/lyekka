#ifndef MAIN_H_INCLUSION_GUARD
#define MAIN_H_INCLUSION_GUARD

#include <string>
#include <vector>
#include <map>
#include "cmd_handler.h"

namespace Lyekka 
{
  

class Main 
{
 public:
  int execute(int argc, char* argv[]);
 protected:
  std::map<std::string, CmdHandler*> handlers;
  

};
 

}

#endif
