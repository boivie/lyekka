#include <iostream>
#include "version_cmd.h"

using namespace std;
using namespace Lyekka;

int VersionCmdHandler::execute(int argc, char* argv[])
{
  cout << PACKAGE << " version " << PACKAGE_VERSION << endl;
  return 0;
}



