#pragma once

#include <stdint.h>
#include <boost/filesystem.hpp>

class Pack {
public:
  Pack(uint64_t num, boost::filesystem::path& fname);
  Pack(const Pack& copy);
  ~Pack();
  int fd() const { return m_fd; };
  uint64_t num() const { return m_num; };
  const boost::filesystem::path& fname() const { return m_fname; }
private:
  int m_fd;
  const uint64_t m_num;
  const boost::filesystem::path m_fname;
};

