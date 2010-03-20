
#define SHA_BITS 256

namespace Lyekka {

class Sha {
public:
  static void base16_decode(unsigned char* dest_p, const char* src_p);
  char* base16(char* buf_p) const;
  void set_base16(const char* base16_p);
  const unsigned char* data(void) const { return m_bytes; }
  unsigned char* mutable_data(void) { return m_bytes; }
  bool operator<(const Sha& other) const { return memcmp(m_bytes, other.m_bytes, SHA_BITS/8) < 0; }
  bool operator==(const Sha& other) const { return memcmp(m_bytes, other.m_bytes, SHA_BITS/8) == 0; }
private:
  unsigned char m_bytes[SHA_BITS/8];
};

}
