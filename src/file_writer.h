#ifndef FILE_WRITER_H_INCLUSION_GUARD
#define FILE_WRITER_H_INCLUSION_GUARD

#include "sha.h"
#include "object_writer.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace Lyekka {

  class FileWriter : public ObjectWriter {
  public:
    FileWriter() : m_fos_p(NULL) {}
    void commit(Sha sha);
    google::protobuf::io::ZeroCopyOutputStream& get_writer();
  private:
    std::auto_ptr<google::protobuf::io::FileOutputStream> m_fos_p;
    char m_tmpname[20];
  };
}

#endif
