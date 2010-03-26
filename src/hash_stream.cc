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

Sha Sha256OutputStream::get_digest()
{
  Sha digest;
  if (m_last_data != NULL)
  {
    SHA256_Update(&m_ctx, (const uint8_t*)m_last_data, m_last_size);
    m_last_data = NULL;
  }
  SHA256_Final(digest.mutable_data(), &m_ctx);
  return digest;
}
