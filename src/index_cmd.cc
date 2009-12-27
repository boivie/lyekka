#include <iostream>
#include <string>
#include "indexer.h"
#include "db.h"
#include "paths.h"
#include "cmd_handler.h"

using namespace std;
using namespace Lyekka;

DECLARE_COMMAND(index, "update-index", "Updates the index");

int CMD_index::execute(int argc, char* argv[])
{
  list<PathInfo> paths = Paths::get();
  Indexer idx(Db::get());
  for (list<PathInfo>::iterator i = paths.begin(); i != paths.end(); ++i)
  {
    idx.update_index(i->path);
  }
}



