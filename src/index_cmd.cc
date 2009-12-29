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
  cout << "Updating index." << endl;
  list<PathInfo> paths = Paths::get();
  Indexer idx(Db::get());
  for (list<PathInfo>::iterator i = paths.begin(); i != paths.end(); ++i)
  {
    idx.update_index(i->path);
  }
  const IndexerStats& stats = idx.get_stats();
  cout << "Done. " 
       << stats.found << " files found, " 
       << stats.added << " added, "
       << stats.updated << " updated and "
       << stats.deleted << " deleted." << endl;
}



