#include <cassert>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include "pack.h"

#ifndef O_NOATIME
# define O_NOATIME 0
#endif

Pack::Pack(uint64_t num, const boost::filesystem::path& fname)
  : m_fname(fname), m_num(num)
{
  int flags = O_RDONLY | O_NOATIME;
  m_fd = open(fname.string().c_str(), flags);
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
