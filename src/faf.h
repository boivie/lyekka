#ifndef FAF_H_INCLUSION_GUARD
#define FAF_H_INCLUSION_GUARD

// Files and folders. 

#include <cstring>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <list>
#include "lyekka.pb.h"

namespace Lyekka {

  class Folder;
  class File;
  class Part;

  typedef boost::shared_ptr<Folder> FolderPtr;
  typedef boost::shared_ptr<File> FilePtr;
  typedef boost::shared_ptr<Part> PartPtr;
  typedef std::list<FolderPtr> FolderList;
  typedef std::list<FilePtr> FileList;
  typedef std::list<PartPtr> PartList;

  class Folder {
  public:
    Folder(const boost::filesystem::path& path) : m_path(path) {}
    virtual ~Folder() {}
    virtual FolderList get_sub_folders() = 0;
    virtual FileList get_files() = 0;
    const Lyekka::pb::TreeRef& pb() const { return m_pb; }
    const boost::filesystem::path& path() const { return m_path; }
  protected:
    Lyekka::pb::TreeRef m_pb;
    const boost::filesystem::path m_path;
  };

  class File {
  public:
    File(const boost::filesystem::path& path) : m_path(path) {}
    virtual ~File() {}
    const Lyekka::pb::FileEntry& pb() const { return m_pb; }
    const boost::filesystem::path& path() const { return m_path; }
    virtual PartList get_parts() = 0;
  protected:
    Lyekka::pb::FileEntry m_pb;
    const boost::filesystem::path m_path;
  };

  class Part {
  public:
    Part(uint64_t offset, uint32_t size) : m_offset(offset), m_size(size) {};
    uint64_t offset() const { return m_offset; }
    uint32_t size() const { return m_size; }
  private:
    uint64_t m_offset;
    uint32_t m_size;
  };
}

#endif
