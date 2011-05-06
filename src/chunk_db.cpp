#include <cassert>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <google/sparse_hash_map>
#include <boost/filesystem.hpp>
#include <boost/smart_ptr.hpp>
#include "chunk_db.h"

using namespace std;
namespace fs = boost::filesystem;

#define PACK_MAX_SIZE ((uint64_t)(500*1024)*1024)

#ifndef O_NOATIME
# define O_NOATIME 0
#endif

fs::path ChunkDatabase::get_idx(const fs::path& pack_fname) const
{
  const std::string index_fname = pack_fname.string() + ".idx";
  return fs::path(index_fname);
}

const StoredChunk ChunkDatabase::find(const ChunkId& cid) {
  ChunkT::const_iterator it = m_chunks.find(cid);
  if (it == m_chunks.end()) {
    throw ChunkNotFound();
  }

  const ChunkLocation loc = it->second;
  PackT::const_iterator pit = m_packs.find(loc.pack());
  assert(pit != m_packs.end());
  assert(pit->second->num() == loc.pack());

  PackPtr pack = pit->second;
  BufferPtr buf(new vector<uint8_t>(loc.size()));
  uint8_t* raw_p = &(*buf)[0];

  lseek(pack->fd(), loc.offset(), SEEK_SET);
  read(pack->fd(), raw_p, loc.size());

  size_t payload_offset = ntohl(*(uint32_t*)(raw_p + 40));
  size_t payload_size = ntohl(*(uint32_t*)(raw_p + 36));
  return StoredChunk(cid, buf, payload_offset, payload_size);
}

bool ChunkDatabase::load() {
  if (has_partial()) {
    cerr << "---- WARNING ----"  << endl
	 << "Partial file found, and --repair not specified." << endl
	 << "---- WARNING ----"  << endl;
    return false;
  }
  find_indexes();
  PackT::const_reverse_iterator rit = m_packs.rbegin();
  if (rit != m_packs.rend())
    m_latest_pack = rit->first;
  load_indexes();
  cout << "Loaded " << m_packs.size() << " packs with "
       << m_chunks.size() << " chunks." << endl;
  return true;
}

void ChunkDatabase::set_path(const boost::filesystem::path& path) {
  m_path = path;
  m_partial = path / fs::path("partial.db");
}

bool ChunkDatabase::has_partial() {
  return boost::filesystem::exists(partial());
}

bool ChunkDatabase::repair() {
  return false;
}

bool ChunkDatabase::create_partial() {
  const fs::path index = get_idx(m_partial);
  int flags = O_WRONLY | O_CREAT | O_EXCL | O_NOATIME;
  int mode = S_IRUSR | S_IWUSR;

  m_partial_w_fd = open(partial().string().c_str(), flags, mode);
  m_partial_idx_fd = open(index.string().c_str(), flags, mode);
		    
  if (m_partial_w_fd == -1 || m_partial_idx_fd == -1) {
    cerr << "Failed to create partial or index file" << endl;
    return false;
  }

  preallocate(m_partial_w_fd);
  // Write pack header.
  const uint8_t pack_superblock[64] =
    {0x32, 0x7d, 0x25, 0x2e, 0x42, 0x18, 0x77, 0x3a,
     0x5b, 0x47, 0xc8, 0x58, 0x4d, 0xf3, 0x10, 0x2d,
     0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x06, 0x2e, 0x13, 0xca,
     0x4a, 0xe3, 0x11, 0xbb, 0x3a, 0x2e, 0xcd, 0xa9,
     0x98, 0xb7, 0x7f, 0x08, 0xbf, 0x37, 0x8f, 0xee};
  const uint8_t index_superblock[32] =
    {0xca, 0xf7, 0xe7, 0xbf, 0x96, 0xa7, 0x99, 0xf0,
     0x54, 0x74, 0x69, 0xcb, 0x69, 0x8b, 0xfb, 0x68,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  write_data(pack_superblock, sizeof(pack_superblock));
  write(m_partial_idx_fd, index_superblock, sizeof(index_superblock));
  m_pack_offset = 64;

  PackPtr p(new Pack(0, m_partial));
  m_packs.insert(std::make_pair(0, p));

  return true;
}

size_t ChunkDatabase::write_data(const void* data_p, size_t len) {
  SHA1_Update(&sha1_ctx, (const uint8_t*)data_p, len);
  write(m_partial_w_fd, data_p, len);
  return len;
}

void ChunkDatabase::write_chunk(const StoredChunk& sc)
{
  ChunkT::const_iterator it = m_chunks.find(sc.cid());
  if (it != m_chunks.end()) {
    cout << "Chunk " << sc.cid().hex() << " already exists." << endl;
    return;
  }

  BufferPtr buf = sc.raw_buffer();
  size_t written = 0;
  const uint8_t chunk_magic[16] = 
    {0x6f, 0x66, 0xac, 0x53, 0x1e, 0xb1, 0x46, 0xef, 
     0xe2, 0xec, 0x95, 0x86, 0x04, 0x32, 0xcb, 0x85};
  struct {
    uint8_t magic[16];
    uint8_t chunk_id[20];
    uint32_t payload_size;
    uint32_t header_size;
    uint32_t flags;
    uint8_t padding[16];
  } header;
  memcpy(header.magic, chunk_magic, 16);
  memcpy(header.chunk_id, sc.cid().binary(), 20);
  header.payload_size = htonl(buf->size());
  header.header_size = htonl(64);
  header.flags = htonl(0);
  memset(header.padding, 0, sizeof(header.padding));
  written += write_data(&header, sizeof(header));
  written += write_data(&(*buf)[0], buf->size());
  const uint8_t padding_array[32] =
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  size_t padding = (32 - buf->size()) % 32;
  if (padding) written += write_data(padding_array, padding);
  written += write_data(padding_array, 20);
  written += write_data(padding_array, 4);
  const uint8_t footer_magic[8] =
    {0x5d, 0x6d, 0xcc, 0x7d, 0xfe, 0x63, 0x1e, 0x4f};
  written += write_data(footer_magic, sizeof(footer_magic));
  struct {
    uint8_t chunk_id[20];
    uint32_t offset;
    uint32_t size;
    uint32_t flags;
  } index_entry;
  memcpy(index_entry.chunk_id, sc.cid().binary(), 20);
  index_entry.offset = htonl(m_pack_offset);
  index_entry.size = htonl(written);
  index_entry.flags = htonl(0);
  write(m_partial_idx_fd, &index_entry, sizeof(index_entry));

  // Written to disk - now serve it to clients
  const ChunkLocation loc(0, m_pack_offset, written);
  m_chunks[sc.cid()] = loc;

  m_pack_offset += written;
}

void ChunkDatabase::close_partial()
{
  const fs::path partial_idx = get_idx(m_partial);

  if (m_pack_offset == 64) {
    // We haven't written any chunks. Just remove the partial file instead.
    cout << "No entries written, removing partial file" << endl;
    fs::remove(m_partial);
    fs::remove(partial_idx);
  } else {
    char basename[100];
    uint8_t sha1_bin[20];
    char sha1_hex[41];

    SHA1_Final(&sha1_ctx, sha1_bin);

    for (int i = 0; i < 20; i++) sprintf(&sha1_hex[i * 2], "%02x", sha1_bin[i]);
    sha1_hex[40] = 0;

    sprintf(basename, "pack-%012llu-%s", m_latest_pack + 1, sha1_hex);

    const fs::path pack_name = m_path / string(basename);
    const fs::path idx_name = get_idx(pack_name);
    cout << "Renaming partial to " << pack_name << endl;
    close(m_partial_w_fd);
    close(m_partial_idx_fd);
    fs::rename(m_partial, pack_name);
    fs::rename(partial_idx, idx_name);
  }
}

void ChunkDatabase::preallocate(int fd) {
  (void)fd;
#if defined(__linux__)
  int err = posix_fallocate(fd, PACK_MAX_SIZE);
  if (err == 0)
    return;

  cerr << "posix_fallocate failed, " << errno << endl;
#endif

#if 0
  /* We don't preallocate during testing */
  off_t filelen = lseek(fd, PACK_MAX_SIZE - 1, SEEK_SET);
  cout << "offset " << filelen << endl;
  if (filelen == PACK_MAX_SIZE - 1) {
    const char onebyte[1] = {0};
    if (write(fd, onebyte, 1) == 1)
      return;
  }
  cout << "Preallocating ..." << endl;
  const size_t len = 1024 * 1024;
  const boost::scoped_array<char> buf_holder(new char[len]);
  char* buf = buf_holder.get();
  memset(buf, 0, len);
  uint64_t left = PACK_MAX_SIZE;
  while (left > 0) {
    size_t actual = left > len ? len : left;
    ssize_t written = write(fd, buf, actual);
    if (written < 0) {
      cerr << "Failed to write, " << errno << endl;
      return;
    }
    left -= written;
  }
  lseek(fd, 0, SEK_SET);
  cout << "Preallocation done." << endl;
#endif
}

void ChunkDatabase::load_indexes() {
  for (PackT::const_iterator i = m_packs.begin(); i != m_packs.end(); ++i) {
    char sb[32];
    char entry[32];
    boost::filesystem::path fname = get_idx(i->second->fname().string());
    FILE* fp = fopen(fname.string().c_str(), "rb");
    if (!fp) {
      cerr << "Failed to open file" << endl;
      continue;
    }

    if (fread(sb, 1, 32, fp) != 32) {
      cerr << "Failed to read index superblock" << endl;
      fclose(fp);
      continue;
    }

    while (fread(entry, 1, 32, fp) == 32) {
      uint32_t offset = ntohl(*(uint32_t*)(entry + 20));
      uint32_t size = ntohl(*(uint32_t*)(entry + 24));
      uint32_t flags = ntohl(*(uint32_t*)(entry + 28));
      (void)flags;
      const ChunkLocation loc(i->first, offset, size);
      const ChunkId cid = ChunkId::from_bin(entry);
      m_chunks[cid] = loc;
    }
  }
}

void ChunkDatabase::find_indexes() {
  DIR *dirp = opendir(m_path.string().c_str());
  struct dirent *dent;

  if (!dirp) {
    cerr << "Couldn't open pack directory" << endl;
    return;
  }

  while ((dent = readdir(dirp)) != NULL) {
    // format: pack-123456789012-{40-sha1} = 62 characters
    if (strlen(dent->d_name) != 58) continue;
    if (!(!strncmp(dent->d_name, "pack-", 5) &&
	  dent->d_name[17] == '-')) continue;
    uint64_t num = (uint64_t)strtoull(dent->d_name + 5, NULL, 10);
    if (num == 0 && errno == EINVAL) continue;

    fs::path fname = m_path / string(dent->d_name);
    PackPtr p(new Pack(num, fname));
    m_packs.insert(std::make_pair(num, p));
  }
 
  closedir(dirp);
}

void ChunkDatabase::dump_all(void) {
  cout << "[[dump begin]]" << endl;
  for (ChunkT::const_iterator it = m_chunks.begin(); it != m_chunks.end(); ++it) {
    cout << it->first << " " << it->second << endl;
  }
  cout << "[[dump end]]" << endl;
}
