#include <string>
#include "lyekka.h"
#include "mmap_stream.h"
#include <sys/mman.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

namespace Lyekka {

MmapInputStream::MmapInputStream(int fd, off_t offset, size_t size)
  : m_close(false), m_fd(fd), m_mmap_p(NULL), m_cur_p(NULL)
{
  if (size == 0) {
    struct stat st;
    if (fstat(fd, &st) == -1) {
      throw RuntimeError();
    }
    size = st.st_size;
  }
  if (size > 0) {
    // Align offset downwards to closest page boundary
    int page_size = getpagesize();
    off_t mmap_offset = (offset / page_size) * page_size;
    m_page_offset = offset - mmap_offset;
    m_size = size + m_page_offset;
    m_mmap_p = mmap(NULL, m_size, PROT_READ, MAP_PRIVATE, fd, mmap_offset);
    if (m_mmap_p == MAP_FAILED) {
      cerr << "mmap failed: " << fd << ", " << m_page_offset << ", " << m_size << " = " << strerror(errno) << endl;
      throw RuntimeError();
    }
    m_cur_p = (char*)m_mmap_p + m_page_offset;
  }
  m_size_left = size;
}

MmapInputStream::~MmapInputStream() 
{
  if (m_mmap_p != NULL)
    munmap(m_mmap_p, m_size);
  if (m_close) {
    close(m_fd);
  }
}

bool MmapInputStream::Next(const void** data, int* size) 
{
  // Return maximum 1 MB of data
  *data = m_cur_p;
  if (m_size_left > 1024*1024) {
    *size = 1024*1024;
  } else {
    *size = m_size_left;
  }
  m_cur_p = m_cur_p + *size;
  m_size_left -= *size;
  //  cout << "Next::size: " << *size << " left: " << m_size_left << endl;
  return *size > 0;
}

void MmapInputStream::BackUp(int count) 
{
  m_cur_p = m_cur_p - count;
  m_size_left += count;
  //  cout << "Backup::count: " << count << " left: " << m_size_left << endl;
}

bool MmapInputStream::Skip(int count) 
{
  m_cur_p = m_cur_p + count;
  m_size_left -= count;
}

google::protobuf::int64 MmapInputStream::ByteCount() const {
  return m_size_left;
}


}
