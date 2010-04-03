#ifndef FILE_SYSTEM_ITERATOR_H_INCLUSION_GUARD
#define FILE_SYSTEM_ITERATOR_H_INCLUSION_GUARD

// Files and folders. 

#include <cstring>
#include <stdexcept>
#include "faf.h"

namespace Lyekka {
  class FileSystemFolder : public Folder {
  public:
    FileSystemFolder(const boost::filesystem::path& dir);
    ~FileSystemFolder() {  }
    FolderList get_sub_folders();
    FileList get_files();
  };

  class FileSystemFile : public File {
  public:
    FileSystemFile(const boost::filesystem::path& file);
    ~FileSystemFile() { }
    PartList get_parts();
  };

  class FileSystemIterator {
  public:
    FileSystemIterator() {};
    FolderPtr iterate(const boost::filesystem::path& dir) { FolderPtr folder_p(new FileSystemFolder(dir)); return folder_p; }
  };
}

#endif
