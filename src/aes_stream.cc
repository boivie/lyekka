#include <iostream>
#include "aes_stream.h"
#include "lyekka.h"
#include <openssl/evp.h>

using namespace Lyekka;
using namespace std;

class OpenSslAes128CbcEngine : public AesEngine {
public:
  OpenSslAes128CbcEngine(const AesKey& key) : AesEngine(key) 
  {
    EVP_CIPHER_CTX_init(&m_ctx);
    EVP_EncryptInit(&m_ctx, EVP_aes_128_cbc(), key.key(), key.iv());
  }
  virtual ~OpenSslAes128CbcEngine() {
    EVP_CIPHER_CTX_cleanup(&m_ctx);
  }
  size_t update(uint8_t* out_p, const uint8_t* in_p, size_t in_len) 
  {
    int out_len = in_len + block_size() - 1;
    EVP_EncryptUpdate(&m_ctx, out_p, &out_len, in_p, in_len);
    return out_len;
  }
  size_t final(uint8_t* out_p) 
  {
    int out_len = block_size();
    EVP_EncryptFinal(&m_ctx, out_p, &out_len);
    return out_len;
  }
  size_t block_size() const {
    return 128/8;
  }

private:
  EVP_CIPHER_CTX m_ctx;
};

AesOutputStream::AesOutputStream(ZeroCopyOutputStream* sub_stream,
				 Mode mode,
				 const AesKey& key)
 : m_copying_output(sub_stream, new OpenSslAes128CbcEngine(key)),
   m_impl(&m_copying_output, AES_OUTPUT_STREAM_BUFFER_SIZE),
   m_closed(false)
{
  assert(key.length() == 128);
  assert(mode == AesOutputStream::CBC);
}

bool AesOutputStream::Next(void** data, int* size)
{
  return m_impl.Next(data, size);
}

void AesOutputStream::BackUp(int count)
{
  m_impl.BackUp(count);
}

void AesOutputStream::Close()
{
  if (!m_closed) {
    m_impl.Flush();
    m_copying_output.Close();
    m_closed = true;
  }
}

void AesOutputStream::CopyingAesOutputStream::Close() {
  uint8_t last_buf[AES_OUTPUT_STREAM_MAX_BLOCK_SIZE];
  int written = m_engine_p->final(last_buf);
  write_to_stream(m_substream, last_buf, written);
}

google::protobuf::int64 AesOutputStream::ByteCount() const
{
  return m_impl.ByteCount();
}

AesOutputStream::CopyingAesOutputStream::
  CopyingAesOutputStream(ZeroCopyOutputStream* substream, 
			 AesEngine* engine_p)
  : m_substream(substream),
    m_engine_p(engine_p)
{
}

bool AesOutputStream::CopyingAesOutputStream
  ::Write(const void* buffer, int size) 
{
  // TODO: can be optimized with some effort by removing the 
  // extra buffer copying.
  assert(size <= AES_OUTPUT_STREAM_BUFFER_SIZE);
  int written = m_engine_p->update(m_tempbuf, (const uint8_t*)buffer, size);

  if (!write_to_stream(m_substream, m_tempbuf, written)) 
    return false;

  return true;
}
  
