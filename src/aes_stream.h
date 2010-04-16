#ifndef AES_STREAM_H_INCLUSION_GUARD
#define AES_STREAM_H_INCLUSION_GUARD

#include <vector>
#include <string>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <memory>

namespace Lyekka {

  class AesKey {
  public:
    virtual const uint8_t* key() const = 0;
    virtual const uint8_t* iv() const = 0;
    virtual size_t length() const = 0;
  };

  class Aes128Key : public AesKey {
  public:
    Aes128Key(const uint8_t* key_p, const uint8_t* iv_p) 
    {
      memcpy(m_key, key_p, sizeof(m_key));
      memcpy(m_iv, iv_p, sizeof(m_iv));
    }
    virtual const uint8_t* key() const { return m_key; }
    virtual const uint8_t* iv() const { return m_iv; }
    virtual size_t length() const { return 128; }
  protected:
    uint8_t m_key[128/8];
    uint8_t m_iv[128/8];
  };

  class AesEngine {
  public:
    AesEngine(const AesKey& key) : m_key(key) {}
    virtual size_t update(uint8_t* out_p, const uint8_t* in_p, size_t in_len) = 0;
    virtual size_t final(uint8_t* out_p) = 0;
    virtual size_t block_size() const = 0;
  protected:
    const AesKey& m_key;
  };

#define AES_OUTPUT_STREAM_BUFFER_SIZE 4096
// 32 == 256 bit encryption.
#define AES_OUTPUT_STREAM_MAX_BLOCK_SIZE 32

  class AesInputStream : public google::protobuf::io::ZeroCopyInputStream {
  public:
    enum Mode {
      CBC = 1,
    };

    explicit AesInputStream(ZeroCopyInputStream* sub_stream,
			    Mode mode,
			    const AesKey& key);
    virtual ~AesInputStream() { }
    
    // implements ZeroCopyInputStream ----------------------------------
    bool Next(const void** data, int* size);
    void BackUp(int count);
    bool Skip(int count);
    google::protobuf::int64 ByteCount() const;
  private:
    class CopyingAesInputStream : public google::protobuf::io::CopyingInputStream {
    public:
      CopyingAesInputStream(ZeroCopyInputStream* substream, 
			    AesEngine* m_engine_p);
      ~CopyingAesInputStream() {};

      // implements CopyingInputStream ---------------------------------
      int Read(void* buffer, int size);
    private:
      std::auto_ptr<AesEngine> m_engine_p;
      google::protobuf::io::ZeroCopyInputStream* m_substream;
      uint8_t m_tempbuf[AES_OUTPUT_STREAM_BUFFER_SIZE
			+ AES_OUTPUT_STREAM_MAX_BLOCK_SIZE];
      bool m_eof;
    };
    
    CopyingAesInputStream m_copying_input;
    google::protobuf::io::CopyingInputStreamAdaptor m_impl;
  };

  class AesOutputStream : public google::protobuf::io::ZeroCopyOutputStream {
  public:
    enum Mode {
      CBC = 1,
    };

    explicit AesOutputStream(ZeroCopyOutputStream* sub_stream,
			     Mode mode,
			     const AesKey& key);
    virtual ~AesOutputStream() { Close(); }
    void Close();
    
    // implements ZeroCopyOutputStream ---------------------------------
    bool Next(void** data, int* size);
    void BackUp(int count);
    google::protobuf::int64 ByteCount() const;

  private:
    class CopyingAesOutputStream : public google::protobuf::io::CopyingOutputStream {
    public:
      CopyingAesOutputStream(ZeroCopyOutputStream* substream, 
			     AesEngine* m_engine_p);
      ~CopyingAesOutputStream() {}
      void Close();
      // implements CopyingOutputStream --------------------------------
      bool Write(const void* buffer, int size);
    private:
      std::auto_ptr<AesEngine> m_engine_p;
      ZeroCopyOutputStream* m_substream;
      uint8_t m_tempbuf[AES_OUTPUT_STREAM_BUFFER_SIZE
			+ AES_OUTPUT_STREAM_MAX_BLOCK_SIZE];
    };

    CopyingAesOutputStream m_copying_output;
    bool m_closed;
    google::protobuf::io::CopyingOutputStreamAdaptor m_impl;    
  };

}
#endif
