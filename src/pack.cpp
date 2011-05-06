#include <cassert>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include "pack.h"

Pack::Pack(uint64_t num, boost::filesystem::path& fname)
  : m_fname(fname), m_num(num)
{
  m_fd = open(fname.string().c_str(), O_RDONLY);
}

extern void die();

Pack::Pack(const Pack& copy)
  : m_num(copy.m_num)
{
  die();
}

Pack::~Pack()
{
  close(m_fd);
}
