#include <arpa/inet.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "manifest.h"
#include "hash_stream.h"
#include <fcntl.h>
#include "lyekka_impl.pb.h"
#include "lyekka.h"
#include <google/protobuf/io/gzip_stream.h>
#include <openssl/rand.h>

using namespace Lyekka;
using namespace std;
using namespace boost;
using namespace google::protobuf::io;

auto_ptr<ObjectIdentifier> Manifest::serialize(ZeroCopyOutputStream* os_p) 
{
  encrypt_secure_props();
  Sha256OutputStream hos(os_p);
  write_to_stream(&hos, "mfst", 4);
  {
    CodedOutputStream cos(&hos);
    uint32_t refs_size = htonl(1);
    cos.WriteRaw(&refs_size, sizeof(refs_size));
    cos.WriteRaw(m_entry_point.data(), SHA_BITS / 8);
    uint32_t pb_size = htonl(m_pb.ByteSize());
    cos.WriteRaw(&pb_size, sizeof(pb_size));
  }
  // The protocol buffer is zlib-compressed
  GzipOutputStream::Options o;
  o.format = GzipOutputStream::ZLIB;
  GzipOutputStream gzos(&hos, o);
  m_pb.SerializeToZeroCopyStream(&gzos);
  gzos.Close();
return std::auto_ptr<ObjectIdentifier>(new Aes128ObjectIdentifier(hos.get_digest(), m_key));
}

void Manifest::create_secure_props_key()
{
  uint8_t buf[128/8];
  RAND_bytes(buf, 128 / 8);
  m_key.set_key(buf);

  RAND_pseudo_bytes(buf, 128 / 8);
  m_key.set_iv(buf);
}

void Manifest::encrypt_secure_props()
{
  string buf;
  buf.reserve(40000);
  StringOutputStream sos(&buf);
  GzipOutputStream::Options o;
  o.format = GzipOutputStream::ZLIB;
  create_secure_props_key();
  AesOutputStream aos(&sos, AesOutputStream::CBC, m_key);
  GzipOutputStream gzos(&aos, o);
  m_secure_pb.SerializeToZeroCopyStream(&gzos);
  gzos.Close();
  aos.Close();
  m_pb.mutable_secure_properties()->set_encrypted_data(buf);
  m_pb.mutable_secure_properties()->set_iv(m_key.iv(), 128/8);
}

auto_ptr<Manifest> Manifest::deserialize(google::protobuf::io::ZeroCopyInputStream* is_p)
{
  auto_ptr<Manifest> manifest_p(new Manifest());
  int32_t pb_size = -1;
  {
    CodedInputStream cis(is_p);
    char header[4];
    cis.ReadRaw(&header, sizeof(header));

    if (memcmp(header, "mfst", 4) != 0) {
      throw InvalidManifestException("Invalid magic");
    }
    
    int32_t refs_size = -1;
    cis.ReadRaw(&refs_size, sizeof(refs_size));
    refs_size = ntohl(refs_size);
    if (refs_size != 1)
      throw InvalidManifestException("Invalid refs size");
    cis.ReadRaw(manifest_p->m_entry_point.mutable_data(), SHA_BITS/8);
    
    cis.ReadRaw(&pb_size, sizeof(pb_size));
    pb_size = ntohl(pb_size);
    if (pb_size < 0)
      throw InvalidManifestException("Invalid pb size");
  }
  
  GzipInputStream gzis(is_p, GzipInputStream::ZLIB);
  if (!manifest_p->m_pb.ParseFromZeroCopyStream(&gzis))
    throw InvalidManifestException("Invalid protocol buffer");

  return auto_ptr<Manifest>(manifest_p.release());
}

void Manifest::add_property(std::string name, std::string value)
{
  pb::Property* prop_p = m_pb.mutable_properties()->add_properties();
  prop_p->set_key(name);
  prop_p->set_str_val(value);
}

void Manifest::add_secure_property(std::string name, std::string value)
{
  pb::Property* prop_p = m_secure_pb.add_properties();
  prop_p->set_key(name);
  prop_p->set_str_val(value);
}

void Manifest::decrypt(const AesKey& key)
{
  m_key.set_key(key.key());
  m_key.set_iv((const uint8_t*)m_pb.secure_properties().iv().c_str());
  const string& buf = m_pb.secure_properties().encrypted_data();
  ArrayInputStream sis(buf.c_str(), buf.length());
  AesInputStream ais(&sis, AesInputStream::CBC, m_key);
  GzipInputStream gzis(&ais, GzipInputStream::ZLIB);
  CodedInputStream decoder(&gzis);
  m_secure_pb.Clear();
  if (!m_secure_pb.ParseFromCodedStream(&decoder))
    throw InvalidManifestException("Invalid key");
}

