#ifndef OBJECT_H_INCLUSION_GUARD
#define OBJECT_H_INCLUSION_GUARD

#include <boost/shared_ptr.hpp>
#include "sha.h"
#include "aes_stream.h"
#include <google/protobuf/io/zero_copy_stream.h>
#include <stdexcept>

namespace Lyekka {

  class NoKeyException : public std::runtime_error {
  public:
    NoKeyException() : std::runtime_error("The object is not encrypted") {};
  };

  class ObjectIdentifier {
  public:
    ObjectIdentifier(const Sha& sha) : m_sha(sha) {}
    const Sha& sha() const { return m_sha; }
    virtual const AesKey& key() const { throw NoKeyException(); }
    virtual bool has_key() const { return false; }
  private:
    const Sha m_sha;
  };

  class Aes128ObjectIdentifier : public ObjectIdentifier {
  public:
    Aes128ObjectIdentifier(const Sha& sha, const Aes128Key& key) 
      : ObjectIdentifier(sha), m_key(key) {}
    const AesKey& key() const { return m_key; }
    bool has_key() const { return true; }
  private:
    Aes128Key m_key;
  };

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
    virtual void commit(const Sha& sha) = 0;
    virtual google::protobuf::io::ZeroCopyOutputStream& get_writer() = 0;
  };
}

#endif
