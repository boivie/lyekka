#ifndef FOLDERMAP_H_INCLUSION_GUARD
#define FOLDERMAP_H_INCLUSION_GUARD
#include <string>
#include "boost/filesystem/path.hpp"

namespace Lyekka {
  class Folder {
  public:
    int id;
    std::string name; 
    struct Folder* parent_p;
    int parent_id;
  };  

  class FolderMap {
  public:
    FolderMap() : list_p(NULL), m_count(0) {}
    ~FolderMap();
    void load(); 
    void build_path(int folder_id, boost::filesystem::path& path);
    typedef void (*WalkerFn)(Folder& folder);
    void walk_post_order(WalkerFn fn);
  private:
    Folder* list_p;
    size_t m_count;
  };
}

#endif
