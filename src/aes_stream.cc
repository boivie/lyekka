#include <iostream>
#include "aes_stream.h"
#include "lyekka.h"
#include <openssl/evp.h>

using namespace Lyekka;
using namespace std;

class OpenSslAes128CbcEncEngine : public AesEngine {
public:
  OpenSslAes128CbcEncEngine(const AesKey& key) : AesEngine(key) 
  {
    EVP_CIPHER_CTX_init(&m_ctx);
    EVP_EncryptInit(&m_ctx, EVP_aes_128_cbc(), key.key(), key.iv());
  }
  virtual ~OpenSslAes128CbcEncEngine() {
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
  size_t block_size() const { return 128/8; }
private:
  EVP_CIPHER_CTX m_ctx;
};

class OpenSslAes128CbcDecEngine : public AesEngine {
public:
  OpenSslAes128CbcDecEngine(const AesKey& key) : AesEngine(key) 
  {
    EVP_CIPHER_CTX_init(&m_ctx);
    EVP_DecryptInit(&m_ctx, EVP_aes_128_cbc(), key.key(), key.iv());
  }
  virtual ~OpenSslAes128CbcDecEngine() {
    EVP_CIPHER_CTX_cleanup(&m_ctx);
  }
  size_t update(uint8_t* out_p, const uint8_t* in_p, size_t in_len) 
  {
    int out_len = in_len + block_size() - 1;
    EVP_DecryptUpdate(&m_ctx, out_p, &out_len, in_p, in_len);
    return out_len;
  }
  size_t final(uint8_t* out_p) 
  {
    int out_len = block_size();
    EVP_DecryptFinal(&m_ctx, out_p, &out_len);
    return out_len;
  }
  size_t block_size() const { return 128/8; }
private:
  EVP_CIPHER_CTX m_ctx;
};

AesInputStream::AesInputStream(ZeroCopyInputStream* sub_stream,
			       Mode mode,
			       const AesKey& key)
 : m_copying_input(sub_stream, new OpenSslAes128CbcDecEngine(key)),
   m_impl(&m_copying_input, AES_OUTPUT_STREAM_BUFFER_SIZE)
{
  assert(key.length() == 128);
  assert(mode == AesInputStream::CBC);
}

AesOutputStream::AesOutputStream(ZeroCopyOutputStream* sub_stream,
				 Mode mode,
				 const AesKey& key)
 : m_copying_output(sub_stream, new OpenSslAes128CbcEncEngine(key)),
   m_impl(&m_copying_output, AES_OUTPUT_STREAM_BUFFER_SIZE),
   m_closed(false)
{
  assert(key.length() == 128);
  assert(mode == AesOutputStream::CBC);
}

bool AesInputStream::Next(const void** data, int* size) {
  return m_impl.Next(data, size);
}

void AesInputStream::BackUp(int count) {
  m_impl.BackUp(count);
}

bool AesInputStream::Skip(int count) {
  return m_impl.Skip(count);
}

google::protobuf::int64 AesInputStream::ByteCount() const {
  return m_impl.ByteCount();
}

bool AesOutputStream::Next(void** data, int* size)
{
  return m_impl.Next(data, size);
}

void AesOutputStream::BackUp(int count)
{
  m_impl.BackUp(count);
}

google::protobuf::int64 AesOutputStream::ByteCount() const
{
  return m_impl.ByteCount();
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

AesOutputStream::CopyingAesOutputStream::
  CopyingAesOutputStream(ZeroCopyOutputStream* substream, 
			 AesEngine* engine_p)
  : m_substream(substream),
    m_engine_p(engine_p) { }

AesInputStream::CopyingAesInputStream::
  CopyingAesInputStream(ZeroCopyInputStream* substream, 
			AesEngine* engine_p)
  : m_substream(substream),
    m_engine_p(engine_p),
    m_eof(false) { }

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

int AesInputStream::CopyingAesInputStream
  ::Read(void* buffer, int size) 
{
  // TODO: can be optimized with some effort by removing the 
  // extra buffer copying.
  if (m_eof) return 0;

  assert(size <= AES_OUTPUT_STREAM_BUFFER_SIZE);
  int actual = read_from_stream(m_tempbuf, m_substream, size);
  //  cout << "read " << size << " -> " << actual << endl;
  if (actual > 0) {
    actual = m_engine_p->update((uint8_t*)buffer, m_tempbuf, actual);
  } 

  if (actual == 0) {
    actual = m_engine_p->final((uint8_t*)buffer);
    m_eof = true;
  }
  //  cout << "crypt " << actual << endl;

  return actual;
}
  
