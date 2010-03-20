#include <iostream>
#include <string>
#include "indexer.h"
#include "db.h"
#include "cmd_handler.h"

using namespace std;
using namespace Lyekka;

static int gen_objects(CommandLineParser& c)
{
  cout << "Generating objects for all items in database." << endl;
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

LYEKKA_COMMAND(gen_objects, "gen-objects", "", "Generates");

