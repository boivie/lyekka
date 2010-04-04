#ifndef ARCHIVE_WRITER_H_INCLUSION_GUARD
#define ARCHIVE_WRITER_H_INCLUSION_GUARD

#include "sha.h"
#include "object_writer.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <boost/filesystem.hpp>
#include <google/protobuf/io/coded_stream.h>
#include <map>

namespace Lyekka {

  class ArchiveObject {
  public:
    Sha sha;
    off_t offset;
    uint32_t size;
  };

  class ArchiveWriter : public ObjectWriter {
  public:
    ArchiveWriter(const boost::filesystem::path& fname);
    ~ArchiveWriter();
    void commit(Sha sha);
    google::protobuf::io::ZeroCopyOutputStream& get_writer();
    void set_entry_point(const Sha& sha) { m_entry_point = sha; }
  private:
    std::auto_ptr<google::protobuf::io::FileOutputStream> m_fos_p;
    char m_tmpname[20];
    int m_fd;
    off_t m_last_offset;
    std::map<Sha, ArchiveObject> m_objects;
    void write_fanout(google::protobuf::io::CodedOutputStream& cos);
    void write_shas(google::protobuf::io::CodedOutputStream& cos);
    void write_offsets(google::protobuf::io::CodedOutputStream& cos);
    void write_entry_point_idx(google::protobuf::io::CodedOutputStream& cos);
    void write_footer_end(off_t offset);
    Sha m_entry_point;
  };
}

#endif
