#include <iostream>
#include "cmd_handler.h"
#include "sdsqlite.h"

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


sd::sqlite CmdHandler::get_db(void)
{
  if (!file_exists("lyekka.db"))
    throw exception();
  
  return sd::sqlite("lyekka.db");
}



