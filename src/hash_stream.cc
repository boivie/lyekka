#include "hash_stream.h"

using namespace Lyekka;
using namespace std;

Sha256OutputStream::Sha256OutputStream(ZeroCopyOutputStream* sub_stream) 
  : m_substream(sub_stream), m_last_data(NULL), m_last_size(0)
{
  SHA256_Init(&m_ctx);
}

bool Sha256OutputStream::Next(void** data, int* size)
{
  bool ret;
  if (m_last_data != NULL)
  {
    SHA256_Update(&m_ctx, (const uint8_t*)m_last_data, m_last_size);
  }
  ret = m_substream->Next(&m_last_data, &m_last_size);
  *data = m_last_data;
  *size = m_last_size;
  return ret; 
}

void Sha256OutputStream::BackUp(int count)
{
  m_substream->BackUp(count);
  m_last_size -= count;
}

google::protobuf::int64 Sha256OutputStream::ByteCount() const
{
  return m_substream->ByteCount();
}

Sha256Digest Sha256OutputStream::get_digest()
{
  Sha256Digest digest;
  if (m_last_data != NULL)
  {
    SHA256_Update(&m_ctx, (const uint8_t*)m_last_data, m_last_size);
    m_last_data = NULL;
  }
  SHA256_Final(digest.m_digest, &m_ctx);
  return digest;
}


std::vector<uint8_t> Sha256Digest::get()
{
  vector<uint8_t> ret;
  ret.reserve(SHA256_DIGEST_LENGTH);
  memcpy(&ret[0], m_digest, SHA256_DIGEST_LENGTH);
  return ret;
}

std::string Sha256Digest::hex()
{
  char hex[SHA256_DIGEST_LENGTH*2+1];
  char* co = hex;
  uint8_t* ci = m_digest;
  static const char hex_digits[] = "0123456789abcdef";

  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    *co++ = hex_digits[(*ci & 0xf0) >> 4];
    *co++ = hex_digits[*ci & 0x0f];
    ci++;
  }

  hex[SHA256_DIGEST_LENGTH*2] = 0;
  return string(hex);
}

