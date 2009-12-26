#ifndef FORMATTER_H_INCLUSION_GUARD
#define FORMATTER_H_INCLUSION_GUARD
#include <string>
#include <stdint.h>

namespace Lyekka {

  class Formatter {
  public:
    static std::string format_size(uint64_t size);
  };
}

#endif
