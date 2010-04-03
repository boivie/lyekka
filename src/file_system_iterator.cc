#include "file_system_iterator.h"
#include <sys/stat.h>
#include "lyekka.h"

using namespace Lyekka;
namespace fs = boost::filesystem;

#if defined(HAVE_STAT64) && !defined(__APPLE__)
typedef struct stat64 statbuf;
#define file_stat(f, s) stat64(f, s)
#else
typedef struct stat statbuf;
#define file_stat(f, s) stat(f, s)
#endif

FileSystemFolder::FileSystemFolder(const boost::filesystem::path& dir) 
  : Folder(dir)
{
  statbuf statr;
  if (file_stat(dir.string().c_str(), &statr) != 0)
    throw RuntimeError();

  m_pb.set_name(dir.leaf().c_str());
  m_pb.set_mode(statr.st_mode & 0777);
  m_pb.set_mtime(statr.st_mtime);
}

FolderList FileSystemFolder::get_sub_folders() {
  FolderList ret;
  fs::directory_iterator end_iter;
  for (fs::directory_iterator dir_itr(m_path);
       dir_itr != end_iter;
       ++dir_itr ) {
    if (fs::is_directory(dir_itr->status())) {
      FolderPtr folder_p(new FileSystemFolder(dir_itr->path()));
      ret.push_back(folder_p);
    }
  }
  return ret;
}

FileList FileSystemFolder::get_files() {
  FileList ret;
  fs::directory_iterator end_iter;
  for (fs::directory_iterator dir_itr(m_path);
       dir_itr != end_iter;
       ++dir_itr ) {
    if (fs::is_regular_file(dir_itr->status())) {
      FilePtr file_p(new FileSystemFile(dir_itr->path()));
      ret.push_back(file_p);
    }
  }
  return ret;
}

FileSystemFile::FileSystemFile(const boost::filesystem::path& file)
  : File(file)
{
  statbuf statr;
  if (file_stat(file.string().c_str(), &statr) != 0)
    throw RuntimeError();

  m_pb.set_name(file.leaf().c_str());
  m_pb.set_mode(statr.st_mode & 0777);
  m_pb.set_mtime(statr.st_mtime);
  m_pb.set_size(statr.st_size);
}

#define MAX_BLOCK_SIZE (30*1024*1024)

PartList FileSystemFile::get_parts() {
  PartList parts;
  
  uint64_t remaining = m_pb.size();
  uint64_t offset = 0;
  while (remaining > 0)
  {
    uint32_t thispart = MIN(remaining, MAX_BLOCK_SIZE);
    PartPtr part(new Part(offset, thispart));
    remaining -= thispart;
    offset += thispart;
    parts.push_back(part);
  }
  return parts;
}
