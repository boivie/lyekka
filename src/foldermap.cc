#include "foldermap.h"
#include "indexed_obj.h"
#include <iostream>
#include "db.h"

using namespace Lyekka;
using namespace std;

size_t get_count(sd::sqlite& db)
{
  sd::sql countq(db);
  size_t count;
  countq << "SELECT COUNT(*) FROM paths";
  countq.step();
  countq >> count;
  return count;
}

static Folder* get_folder(size_t id, Folder* list_p, size_t count)
{
  size_t low = 0, high = count;

  while (low <= high)
  {
    int mid = (low + high) / 2;
    if (id < list_p[mid].id)
      high = mid - 1;
    else if (id > list_p[mid].id)
      low = mid + 1;
    else 
    {
      return &list_p[mid];
    }
  }
  return NULL;
}

void FolderMap::load(void)
{
  sd::sqlite& db = Db::get();
  m_count = get_count(db);
  list_p = new Folder[m_count]();

  sd::sql selq(db);
  selq << "SELECT id, name, parent FROM paths ORDER BY id";
  size_t i;

  for (i = 0; i < m_count; i++)
  {
    Folder& folder = list_p[i];
    folder.parent_p = NULL;
    if (selq.step())
    {
      selq >> folder.id >> folder.name >> folder.parent_id;
    }
  }

  // Set the parent_p pointers
  for (i = 0; i < m_count; i++)
  {
    Folder& folder = list_p[i];
    if (folder.parent_id == 0) 
      continue;
    struct Folder* parent_p = get_folder(folder.parent_id, list_p, m_count);
    folder.parent_p = parent_p;
    if (parent_p != NULL)
    {
      struct Folder* prev_child_p = parent_p->first_child_p;
      parent_p->first_child_p = &folder;
      folder.sibling_p = prev_child_p;
    }
  }
}

static void recurse_build(Folder* folder_p, boost::filesystem::path& path)
{
  if (folder_p->parent_p != NULL) 
    recurse_build(folder_p->parent_p, path);
  path /= folder_p->name;
} 

void FolderMap::build_path(int folder_id, boost::filesystem::path& path)
{
  Folder* folder_p = get_folder(folder_id, list_p, m_count);
  if (folder_p != NULL)
    recurse_build(folder_p, path);
}

void visit_folder(Folder& folder, WalkerFn walker)
{
  Folder* child_p;
  // Visit children first.
  for (child_p = folder.first_child_p; child_p != NULL; child_p = child_p->sibling_p)
  {
    visit_folder(*child_p, walker);
  }
  // We call the callback after traversing children, since we are in post-order-walk mode.
  walker(folder);
}

void FolderMap::walk_post_order(WalkerFn walker)
{
  for (int i = 0; i < m_count; i++)
  {
    Folder& folder = list_p[i]; 
    if (folder.parent_id == 0)
    {
      visit_folder(folder, walker);
    } 
  }
}
FolderMap::~FolderMap()
{
  
}
