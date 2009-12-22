#include <iostream>
#include "cmd_handler.h"
#include "sdsqlite/sdsqlite.h"

using namespace std;
using namespace Lyekka;

static int file_exists(string filename)
{
  if (FILE* file = fopen(filename.c_str(), "r"))
  {
    fclose(file);
    return true;
  }
  return false;
}


