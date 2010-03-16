#ifndef FOLDERMAP_H_INCLUSION_GUARD
#define FOLDERMAP_H_INCLUSION_GUARD
#include <string>
#include "boost/filesystem/path.hpp"
#include "chunk.h"

namespace Lyekka {
  class Folder {
  public:
    Folder() : parent_p(0), first_child_p(0), sibling_p(0), has_chunk(false) {}
    int id;
    std::string name; 
    struct Folder* parent_p;
    struct Folder* first_child_p;
    struct Folder* sibling_p;
    int parent_id;
    bool has_chunk;
    const Chunk& get_chunk(void);
    void set_chunk(const Chunk& chunk);
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
