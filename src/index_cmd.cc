#include <iostream>
#include <string>
#include "index_cmd.h"
#include "indexer.h"
#include "db.h"
#include "paths.h"

using namespace std;
using namespace Lyekka;


int IndexCmdHandler::execute(int argc, char* argv[])
{
  list<PathInfo> paths = Paths::get();
  Indexer idx(Db::get());
  for (list<PathInfo>::iterator i = paths.begin(); i != paths.end(); ++i)
  {
    idx.update_index(i->path);
  }
}



