#include <iostream>
#include "archive_writer.h"
#include "lyekka.h"
#include <unistd.h>
#include <fcntl.h>

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;

// TODO: Support for entrypoint

void ArchiveWriter::commit(Sha sha) 
{
  m_fos_p->Flush();
  m_fos_p.release();
  off_t this_offset = lseek(m_fd, 0, SEEK_CUR);
  std::map<Sha, ArchiveObject>::iterator i = m_objects.find(sha);
  if (i == m_objects.end()) {
    ArchiveObject ao;
    ao.sha = sha;
    ao.offset = m_last_offset;
    ao.size = this_offset - m_last_offset - 4;
    // Patch the size, which is just in front of this object.
    lseek(m_fd, m_last_offset, SEEK_SET);
    uint32_t size = htonl(ao.size);
    write(m_fd, &size, 4);
    lseek(m_fd, this_offset, SEEK_SET);
    m_objects.insert(std::pair<Sha, ArchiveObject>(sha, ao));
    m_last_offset = this_offset;
  } else {
    // Already present. Roll-back.
    lseek(m_fd, m_last_offset, SEEK_SET);
  }
}

static off_t align_by_four(int fd) 
{
  off_t offset = lseek(fd, 0, SEEK_CUR);
  int extra_bytes = offset % 4;
  if (extra_bytes != 0) {
    write(fd, "\0\0\0", extra_bytes);
    offset += extra_bytes;
  }
  return offset;
}

google::protobuf::io::ZeroCopyOutputStream& ArchiveWriter::get_writer()
{
  m_last_offset = align_by_four(m_fd);
  // reserve space for the size
  write(m_fd, "----", 4);
  m_fos_p.reset(new FileOutputStream(m_fd));
  return *m_fos_p;
}

ArchiveWriter::ArchiveWriter(const boost::filesystem::path& fname)
  : m_fos_p(NULL)
{
  m_fd = xopen(fname.string().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
  write(m_fd, "arch", 4);
  m_last_offset = 4;
}

void ArchiveWriter::write_fanout(CodedOutputStream& cos) 
{
  std::map<Sha, ArchiveObject>::iterator it = m_objects.begin();
  int idx = 0;
  uint32_t fanout[256];
  for (int i = 0; i < 256; i++) {
    while (it != m_objects.end() && i == it->first.data()[0])  {
      ++it;
      idx++;
    }
    fanout[i] = htonl(idx);
  }
  cos.WriteRaw(&fanout, sizeof(fanout));
}

void ArchiveWriter::write_shas(CodedOutputStream& cos) 
{
  for (std::map<Sha, ArchiveObject>::iterator it = m_objects.begin();
       it != m_objects.end();
       ++it) {
    cos.WriteRaw(it->first.data(), 256/8);
  }
}

void ArchiveWriter::write_offsets(CodedOutputStream& cos) 
{
  for (std::map<Sha, ArchiveObject>::iterator it = m_objects.begin();
       it != m_objects.end();
       ++it) {
    // TODO: Add support for 64-bit offsets
    uint32_t offset = htonl(it->second.offset);
    cos.WriteRaw(&offset, 4);
  }
}

void ArchiveWriter::write_entry_point_idx(CodedOutputStream& cos) 
{
  int idx = 0;
  std::map<Sha, ArchiveObject>::iterator ep_it = m_objects.find(m_entry_point);
  for (std::map<Sha, ArchiveObject>::iterator it = m_objects.begin();
       it != ep_it;
       ++it) {
    idx++;
  }
  uint32_t ep_idx = htonl(idx);
  cos.WriteRaw(&ep_idx, 4);
}
 

ArchiveWriter::~ArchiveWriter() {
  if (m_fd != -1) {
    off_t footer_start = align_by_four(m_fd);
    // Allocate size for the footer size.
    write(m_fd, "----aidx", 8);
    {
      FileOutputStream fos(m_fd);
      CodedOutputStream cos(&fos);
      write_fanout(cos);
      write_shas(cos);
      write_offsets(cos);
      write_entry_point_idx(cos);
    }
    // Finalize archive index.
    off_t footer_end = lseek(m_fd, 0, SEEK_CUR);
    uint32_t footer_size = htonl(footer_end - footer_start);
    write(m_fd, &footer_size, 4);
    write(m_fd, "xdia", 4);
    lseek(m_fd, footer_start, SEEK_SET);
    write(m_fd, &footer_size, 4);
    close(m_fd);
  }
}
