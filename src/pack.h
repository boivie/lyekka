#pragma once

#include <stdint.h>

class Pack {
public:
  Pack(uint64_t num, int fd) : m_fd(fd), m_num(num) {};
  int fd() const { return m_fd; };
  uint64_t num() const { return m_num; };
private:
  int m_fd;
  const uint64_t m_num;
};

