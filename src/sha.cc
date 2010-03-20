#include <string.h>
#include <cassert>
#include "sha.h"

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

char base16char(char in)
{
  switch(in) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    return in - '0';
  case 'A': case 'B': case 'C':
  case 'D': case 'E': case 'F':
    return (in - 'A') + 10;
  case 'a': case 'b': case 'c':
  case 'd': case 'e': case 'f':
    return in - 'a' + 10;
  default:
    // TODO: ERROR
    return 0;
  }
}

void Sha::base16_decode(unsigned char* dest_p, const char* src_p)
{
  size_t length = strlen(src_p);
  int i;

  assert(length == 256/4);

  for (i = 0; i < length / 2; i++) {
    dest_p[i] = 
      base16char(*(src_p + 2*i + 0)) << 4 |
      base16char(*(src_p + 2*i + 1));
  }
}


void Sha::set_base16(const char* base16_p)
{
  base16_decode(m_bytes, base16_p);
}
