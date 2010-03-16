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
  {
    snprintf(buf, sizeof(buf), "%u.%2.2u MiB", (unsigned int)(size >> 20), (unsigned int)((size & ((1 << 20) - 1)) * 100) >> 20);
  }
  else
    snprintf(buf, sizeof(buf), "%u.%2.2u GiB", (unsigned int)(size >> 30), (unsigned int)(size & ((1 << 30) - 1)) / 10737419);

  return string(buf);
}

string Formatter::format_rate(uint64_t rate)
{
  char buf[30];

	if (rate > 1 << 10) {
		uint64_t x = rate + 5;  /* for rounding */
		snprintf(buf, sizeof(buf), "%u.%2.2u MiB/s",
			 x >> 10, ((x & ((1 << 10) - 1)) * 100) >> 10);
	} else if (rate)
		snprintf(buf, sizeof(buf), "%u KiB/s", rate);
  return string(buf);
}
