#ifndef OBJECT_H_INCLUSION_GUARD
#define OBJECT_H_INCLUSION_GUARD

#include <boost/shared_ptr.hpp>
#include "sha.h"
#include <google/protobuf/io/zero_copy_stream.h>

namespace Lyekka {

  class ObjectInputStream : public google::protobuf::io::ZeroCopyInputStream {
  public:
    ObjectInputStream(const Sha& sha) : m_sha(sha) {}
    const Sha& sha() const { return m_sha; }
  protected:
    Sha m_sha;
  };

  class ObjectReader { 
  public:
    virtual boost::shared_ptr<ObjectInputStream> find(const Sha& sha) const = 0;
  };

  class ObjectWriter {
  public:
    virtual ~ObjectWriter() {};
    virtual void commit(Sha sha) = 0;
    virtual google::protobuf::io::ZeroCopyOutputStream& get_writer() = 0;
  };
}

#endif
