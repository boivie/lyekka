#ifndef SHA_H_INCLUSION_GUARD
#define SHA_H_INCLUSION_GUARD
#define SHA_BITS 256

#include <string>
#include <iostream>

namespace Lyekka {

typedef char ShaBase16Buf[SHA_BITS/4 + 1];

class Sha {
public:
  friend std::ostream& operator<<(std::ostream& os, const Sha& sha);
  static void base16_decode(unsigned char* dest_p, const char* src_p);
  char* base16(char* buf_p) const;
  void set_base16(const char* base16_p);
  const unsigned char* data(void) const { return m_bytes; }
  unsigned char* mutable_data(void) { return m_bytes; }
  std::string mutable_string(void) const { std::string a; a.insert(0, (const char*)m_bytes, SHA_BITS/8); return a; }
  bool operator<(const Sha& other) const { return memcmp(m_bytes, other.m_bytes, SHA_BITS/8) < 0; }
  bool operator==(const Sha& other) const { return memcmp(m_bytes, other.m_bytes, SHA_BITS/8) == 0; }

private:
  unsigned char m_bytes[SHA_BITS/8];
};

}

#endif
