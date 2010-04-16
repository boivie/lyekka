#include <string.h>
#include <cassert>
#include "sha.h"
#include "lyekka.h"

using namespace Lyekka;

char* Sha::base16(char* buf_p) const
{
  return base16_encode(buf_p, m_bytes, sizeof(m_bytes));
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
