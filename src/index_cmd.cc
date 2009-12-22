#include <iostream>
#include <string>
#include "index_cmd.h"
#include "indexer.h"
#include "db.h"

using namespace std;
using namespace Lyekka;


int IndexCmdHandler::execute(int argc, char* argv[])
{
  Indexer i(Db::get());
  string name(argv[2]);
  i.update_index(name);
}



