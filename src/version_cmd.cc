#include <iostream>
#include "cmd_handler.h"

using namespace std;
using namespace Lyekka;

DECLARE_COMMAND(version, "version", "Shows the version");

int CMD_version::execute(int argc, char* argv[])
{
  cout << PACKAGE << " version " << PACKAGE_VERSION << endl;
  return 0;
}



