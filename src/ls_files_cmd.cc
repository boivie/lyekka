#include <iostream>
#include <string>
#include "cmd_handler.h"
#include "foldermap.h"
#include "db.h"
#include "boost/filesystem/path.hpp"

using namespace std;
using namespace Lyekka;

static int ls_files(CommandLineParser& c)
{
  c.parse_options();
  FolderMap fm;
  fm.load();
  sd::sqlite& db = Db::get();
  
  sd::sql query(db);
  query << "SELECT name, parent FROM files ORDER BY parent";
  while (query.step())
  {
    string name;
    int64_t parent;
    query >> name >> parent;
    boost::filesystem::path path;
    fm.build_path(parent, path);
    path /= name; 
    cout << path << endl;
  }

  return 0;
}

LYEKKA_COMMAND(ls_files, "ls-files", "", "Lists files from index");

