#include <iostream>
#include <string>
#include "indexer.h"
#include "db.h"
#include "cmd_handler.h"

using namespace std;
using namespace Lyekka;

static int index(CommandLineParser& c)
{
  cout << "Updating index." << endl;
  Indexer idx(Db::get());
  idx.update();
  const IndexerStats& stats = idx.get_stats();
  cout << "Done. " 
       << stats.found << " files found, " 
       << stats.added << " added, "
       << stats.updated << " updated and "
       << stats.deleted << " deleted." << endl;
  return 0;
}

LYEKKA_COMMAND(index, "update-index", "", "Updates the index");

