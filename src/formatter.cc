#include "formatter.h"

using namespace Lyekka;
using namespace std;

string Formatter::format_size(uint64_t size)
{
  char buf[30];

  if (size < (1 << 10))
    snprintf(buf, sizeof(buf), "%u bytes", (unsigned int)size);
  else if (size < (1 << 20))
    snprintf(buf, sizeof(buf), "%u KiB", (unsigned int)(size >> 10));
  else if (size < (1 << 30))
    snprintf(buf, sizeof(buf), "%u MiB", (unsigned int)(size >> 20));
  else
    snprintf(buf, sizeof(buf), "%u GiB", (unsigned int)(size >> 30));

  return string(buf);
}


