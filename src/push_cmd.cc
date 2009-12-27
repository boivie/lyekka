#include <iostream>
#include "cmd_handler.h"

using namespace std;
using namespace Lyekka;

DECLARE_COMMAND(push, "push", "Copies the latest data.");

int CMD_push::execute(int argc, char* argv[])
{
  return 0;
}

