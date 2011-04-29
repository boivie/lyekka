#include "chunk_id.h"
using namespace std;

static int base16_decode(int c) {
  switch (c) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    return c - '0';
  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    return c - 'a' + 10;
  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    return c - 'A' + 10;
  default:
    return 0;
  }
}

ChunkId ChunkId::from_b16(const char* data) {
  ChunkId cid;

  for (int i = 0; i < 20; i++) {
    int c1 = base16_decode(*(data + 2*i));
    int c2 = base16_decode(*(data + 2*i + 1));
    cid.cid[i] = c1 << 4 | c2;
  }
  return cid;
}

ChunkId ChunkId::from_bin(const char* data) {
  ChunkId cid;
  memcpy(cid.cid, data, 20);
  return cid;
}

ostream& operator << (ostream& os, const ChunkId& C)
{
  char hex[41];

  for (int i = 0; i < 20; i++)
    sprintf((hex + i * 2), "%02x", C.cid[i]);
  hex[40] = 0;

  os << "<chunk " << hex << ">";
  return os;
}
