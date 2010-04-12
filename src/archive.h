#ifndef ARCHIVE_READER_H_INCLUSION_GUARD
#define ARCHIVE_READER_H_INCLUSION_GUARD

#include "sha.h"
#include "mmap_stream.h"
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <boost/filesystem.hpp>
#include <map>
#include "object.h"

namespace Lyekka {
class ArchiveReader;

class ArchiveObjectInputStream : public ObjectInputStream {
public:
  virtual ~ArchiveObjectInputStream();
  // implements ZeroCopyInputStream ---------------------------------
  bool Next(const void** data, int* size) { return m_is.Next(data, size); }
  void BackUp(int count) { m_is.BackUp(count); }
  bool Skip(int count) { return m_is.Skip(count); }
  google::protobuf::int64 ByteCount() const { return m_is.ByteCount(); }
private:
  friend class ArchiveReader;
  ArchiveObjectInputStream(int fd, off_t offset, size_t size, const Sha& sha) : 
    m_is(fd, offset, size), ObjectInputStream(sha) {}
  MmapInputStream m_is;
};

class ArchiveReader : public ObjectReader {
public:
  ArchiveReader(const boost::filesystem::path& path);
  ~ArchiveReader();
  int entry_count() const { return m_entries; }
  boost::shared_ptr<ObjectInputStream> find(const Sha& sha) const;
  boost::shared_ptr<ArchiveObjectInputStream> find_by_idx(int idx) const;
  Sha entry_point() const { return sha_by_idx(m_entry_point); }
  int entry_point_idx() const { return m_entry_point; }
private:
  Sha sha_by_idx(uint32_t idx) const;
  void* m_mmap_p;  
  const char* m_index_p;
  size_t m_mmap_len;
  int m_entries;
  int m_fd;
  int m_entry_point;
};

class ArchiveWriter : public ObjectWriter {
public:
  ArchiveWriter(const boost::filesystem::path& fname);
  ~ArchiveWriter();
  void commit(Sha sha);
  google::protobuf::io::ZeroCopyOutputStream& get_writer();
  void set_entry_point(const Sha& sha) { m_entry_point = sha; }
private:
  class StoredObject {
  public:
    Sha sha;
    off_t offset;
    size_t size;
  };
  std::auto_ptr<google::protobuf::io::FileOutputStream> m_fos_p;
  char m_tmpname[20];
  int m_fd;
  off_t m_last_offset;
  std::map<Sha, StoredObject> m_objects;
  void write_fanout(google::protobuf::io::CodedOutputStream& cos);
  void write_shas(google::protobuf::io::CodedOutputStream& cos);
  void write_offsets(google::protobuf::io::CodedOutputStream& cos);
  void write_entry_point_idx(google::protobuf::io::CodedOutputStream& cos);
  void write_footer_end(off_t offset);
  Sha m_entry_point;
};

}
#endif
