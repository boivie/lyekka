#include <iostream>
#include "archive.h"
#include "lyekka.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;

void ArchiveWriter::commit(Sha sha) 
{
  m_fos_p->Flush();
  m_fos_p.release();
  off_t this_offset = lseek(m_fd, 0, SEEK_CUR);
  std::map<Sha, StoredObject>::iterator i = m_objects.find(sha);
  if (i == m_objects.end()) {
    StoredObject ao;
    ao.sha = sha;
    ao.offset = m_last_offset;
    ao.size = this_offset - m_last_offset - 4;
    // Patch the size, which is just in front of this object.
    lseek(m_fd, m_last_offset, SEEK_SET);
    uint32_t size = htonl(ao.size);
    write(m_fd, &size, 4);
    lseek(m_fd, this_offset, SEEK_SET);
    m_objects.insert(std::pair<Sha, StoredObject>(sha, ao));
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
  std::map<Sha, StoredObject>::iterator it = m_objects.begin();
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
  for (std::map<Sha, StoredObject>::iterator it = m_objects.begin();
       it != m_objects.end();
       ++it) {
    cos.WriteRaw(it->first.data(), 256/8);
  }
}

void ArchiveWriter::write_offsets(CodedOutputStream& cos) 
{
  for (std::map<Sha, StoredObject>::iterator it = m_objects.begin();
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
  std::map<Sha, StoredObject>::iterator ep_it = m_objects.find(m_entry_point);
  for (std::map<Sha, StoredObject>::iterator it = m_objects.begin();
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

ArchiveReader::ArchiveReader(const boost::filesystem::path& path) {
  m_fd = xopen(path.string().c_str(), O_RDONLY);
  char header[4];
  if (read(m_fd, header, 4) != 4) {
    cerr << "Failed to read header" << endl;
    close(m_fd);
    throw RuntimeError();
  }
  if (memcmp(header, "arch", 4) != 0) {
    cerr << "Not an archive" << endl;
    close(m_fd);
    throw RuntimeError();
  }
  // Read footer and validate.
  off_t end = lseek(m_fd, -8, SEEK_END);
  struct {
    uint32_t length;
    char magic[4];
  } footer;

  if (read(m_fd, &footer, 8) != 8) {
    cerr << "Failed to read footer" << endl;
    close(m_fd);
    throw RuntimeError();
  }
  footer.length = ntohl(footer.length);
  if (memcmp(footer.magic, "xdia", 4) != 0 ||
      (footer.length > end)) {
    cerr << "The archive is corrupt (wrong footer)" << endl;
    close(m_fd);
    throw RuntimeError();
  }
  
  off_t footer_start = lseek(m_fd, end-footer.length, SEEK_SET);
  footer_start += 8;
  struct {
    uint32_t length;
    char magic[4];
  } footer_header;

  if (read(m_fd, &footer_header, 8) != 8) {
    cerr << "Failed to read start of footer" << endl;
    close(m_fd);
    throw RuntimeError();
  }
  if (memcmp(footer_header.magic, "aidx", 4) != 0 ||
      (ntohl(footer_header.length) != footer.length)) {
    cerr << "The archive is corrupt (wrong start of footer)" << endl;
    close(m_fd);
    throw RuntimeError();
  }

  // Passed sanity tests. Now memory map the index and work from there.
  // NOTE: We don't want to memory map the entire file, as it may be larger
  // than the amount of virtual memory we have.
  int page_size = getpagesize();
  off_t mmap_start = footer_start / page_size * page_size;
  m_mmap_p = xmmap(NULL, footer.length, PROT_READ, MAP_PRIVATE, m_fd, mmap_start);
  if (m_mmap_p == MAP_FAILED) {
    cerr << "Failed to mmap" << endl;
    close(m_fd);
    throw RuntimeError();
  }
  m_mmap_len = footer.length;
  m_index_p = (const char*)m_mmap_p + (footer_start - mmap_start);
  m_entries = ntohl(((uint32_t*)m_index_p)[255]);
  m_entry_point = ntohl(*(uint32_t*)(m_index_p + 1024 + m_entries * (256/8 + 4)));
  if (m_entry_point >= m_entries) {
    cerr << "Invalid entry point" << endl;
    close(m_fd);
    throw RuntimeError();
  }
}

ArchiveReader::~ArchiveReader() {
  close(m_fd);
}

boost::shared_ptr<ObjectInputStream> ArchiveReader::find(const Sha& sha) const 
{
  uint32_t* fanout_p = (uint32_t*)m_index_p;
  int fanout_idx = sha.data()[0];
  int lo = (fanout_idx == 0) ? 0 : fanout_p[fanout_idx - 1];
  int hi = fanout_p[fanout_idx];
  
  while (lo < hi) {
    int mid = lo + (hi - lo) / 2;
    int c = memcmp(sha.data(), m_index_p + 1024 + mid * (256/8), 256/8);
    if (c < 0) {
      lo = mid + 1;
    } else if (c > 0) {
      hi = mid;
    } else {
      return find_by_idx(mid);
    }
  }

  cerr << "Not found" << endl;
  throw RuntimeError();
}

boost::shared_ptr<ArchiveObjectInputStream> ArchiveReader::find_by_idx(int idx) const 
{
  Sha sha;
  off_t offset;
  if (idx >= m_entries) {
    throw RuntimeError();
  }

  memcpy(sha.mutable_data(), m_index_p + 1024 + idx * (256 / 8), 256/8);
  offset = ntohl(*(uint32_t*)(m_index_p + 1024 + m_entries * (256 / 8) + idx * 4));
  uint32_t size;
  if (lseek(m_fd, offset, SEEK_SET) == -1) {
    throw RuntimeError();
  }
  if (read(m_fd, &size, 4) != 4) {
    throw RuntimeError();
  }
  boost::shared_ptr<ArchiveObjectInputStream> aois(new ArchiveObjectInputStream(m_fd, offset, ntohl(size), sha));
  return aois;
}


ArchiveObjectInputStream::~ArchiveObjectInputStream() 
{

}

