#ifndef OBJECT_WRITER_H_INCLUSION_GUARD
#define OBJECT_WRITER_H_INCLUSION_GUARD

#include <boost/shared_ptr.hpp>
#include <google/protobuf/io/zero_copy_stream.h>

namespace Lyekka {

  class ObjectWriter {
  public:
    virtual ~ObjectWriter() {};
    virtual void commit(Sha sha) = 0;
    virtual google::protobuf::io::ZeroCopyOutputStream& get_writer() = 0;
  };
}

#endif
