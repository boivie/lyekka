#ifndef FOLDERMAP_H_INCLUSION_GUARD
#define FOLDERMAP_H_INCLUSION_GUARD
#include <string>
#include "boost/filesystem/path.hpp"
#include "objref.h"

namespace Lyekka {
  class FolderMap;

  class FMFolder {
  public:
    int id;
    std::string name; 
    FMFolder* parent_p;
    FMFolder* first_child_p;
    FMFolder* sibling_p;
    int parent_id;
    bool has_obj_ref;
    const ObjRef& obj_ref(void) { return m_obj_ref; }
    void set_obj_ref(const ObjRef& obj) { m_obj_ref = obj; }
    const boost::filesystem::path get_path() const;
  private:
    friend class FolderMap;
    FMFolder();
    ObjRef m_obj_ref;
    FolderMap* m_fm_p;
  };  

  typedef void (*WalkerFn)(FMFolder& folder);

  class FolderMap {
  public:
    FolderMap() : list_p(NULL), m_count(0) {}
    ~FolderMap();
    void load(); 
    void build_path(int folder_id, boost::filesystem::path& path);
    void walk_post_order(WalkerFn fn);
  private:
    FMFolder* list_p;
    size_t m_count;
  };
}

#endif
