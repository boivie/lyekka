#include <string.h>
#include <cassert>
#include "sha.h"
#include "lyekka.h"

using namespace Lyekka;

char* Sha::base16(char* buf_p) const
{
  static const char translate[] = "0123456789abcdef";

  for (int i = 0; i < SHA_BITS/8; i++)
  {
    buf_p[2 * i + 0] = translate[(m_bytes[i] & 0xF0) >> 4];
    buf_p[2 * i + 1] = translate[(m_bytes[i] & 0x0F)];
  }
  buf_p[256/4] = 0;
  return buf_p;
}

void Sha::set_base16(const char* base16_p)
{
  size_t length = strlen(base16_p);
  assert(length == 256/4);

  base16_decode(m_bytes, base16_p);
}

namespace Lyekka {
std::ostream& operator<<(std::ostream& os, const Sha& sha)
{
  char buf[256 / 4 + 1];
  os << sha.base16(buf);
  return os;
}
}
