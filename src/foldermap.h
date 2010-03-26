#ifndef FOLDERMAP_H_INCLUSION_GUARD
#define FOLDERMAP_H_INCLUSION_GUARD
#include <string>
#include "boost/filesystem/path.hpp"
#include "objref.h"

namespace Lyekka {
  class Folder {
  public:
    Folder() : parent_p(0), first_child_p(0), sibling_p(0), has_obj_ref(false) {}
    int id;
    std::string name; 
    Folder* parent_p;
    Folder* first_child_p;
    Folder* sibling_p;
    int parent_id;
    bool has_obj_ref;
    const ObjRef& obj_ref(void) { return m_obj_ref; }
    void set_obj_ref(const ObjRef& obj) { m_obj_ref = obj; }
  private:
    ObjRef m_obj_ref;
  };  

  typedef void (*WalkerFn)(Folder& folder);

  class FolderMap {
  public:
    FolderMap() : list_p(NULL), m_count(0) {}
    ~FolderMap();
    void load(); 
    void build_path(int folder_id, boost::filesystem::path& path);
    void walk_post_order(WalkerFn fn);
  private:
    Folder* list_p;
    size_t m_count;
  };
}

#endif
