#ifndef MANIFEST_H_INCLUSION_GUARD
#define MANIFEST_H_INCLUSION_GUARD

#include <string>
#include <iostream>
#include "sha.h"
#include "lyekka.pb.h"
#include <boost/shared_ptr.hpp>
#include "aes_stream.h"
#include "object.h"

namespace Lyekka {

class InvalidManifestException
{
public:
  InvalidManifestException(const std::string extra = "") : m_extra(extra) {};
  const char* what() { return m_extra.c_str(); }
private:
  const std::string m_extra;
};

class Manifest {
public:
  Manifest(void) {}
  void add_property(std::string name, std::string value);
  void add_secure_property(std::string name, std::string value);
  void set_entry_point(const Sha& sha) { m_entry_point = sha; }
  const Lyekka::pb::Manifest& pb() const { return m_pb; }
  const Lyekka::pb::SecureProperties& secure_properties() const { return m_secure_pb; }

  std::auto_ptr<ObjectIdentifier> serialize(google::protobuf::io::ZeroCopyOutputStream* os_p);
  static std::auto_ptr<Manifest> deserialize(google::protobuf::io::ZeroCopyInputStream* is_p);
  const Sha& entry_point() const { return m_entry_point; }
  void decrypt(const AesKey& key);
private:
  Aes128Key m_key;
  void create_secure_props_key();
  void encrypt_secure_props();
  Lyekka::pb::Manifest m_pb;
  Lyekka::pb::SecureProperties m_secure_pb;
  Sha m_entry_point;
};



}

#endif
