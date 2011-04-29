#include <cassert>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include "chunk_db.h"

using namespace std;

ChunkFindResult ChunkDatabase::find(const ChunkId& cid) {
  ChunkT::const_iterator it = m_chunks.find(cid);
  if (it == m_chunks.end()) {
    assert(0);
  }
  PackT::const_iterator pit = m_packs.find(it->second.pack());
  assert(pit != m_packs.end());
  assert(pit->second.num() == it->second.pack());
  return ChunkFindResult(*this, pit->second, it->second);
}

void ChunkDatabase::load_indexes(const std::string& path) {
  for (PackT::const_iterator i = m_packs.begin(); i != m_packs.end(); ++i) {
    char sb[32];
    char fname[PATH_MAX + 1];
    char entry[32];
    sprintf(fname, "%spack-%012lld.idx", path.c_str(), i->first);
    
    FILE* fp = fopen(fname, "rb");
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
      ChunkLocation loc(i->first, offset, size);
      ChunkId cid = ChunkId::from_bin(entry);
      m_chunks.insert(std::make_pair(cid, loc));
    }
  }
}

void ChunkDatabase::find_indexes(const std::string& path) {
  DIR *dirp = opendir(path.c_str());
  char fname[PATH_MAX + 1];
  struct dirent *dent;

  if (!dirp) {
    cerr << "Couldn't open pack directory" << endl;
    return;
  }

  while ((dent = readdir(dirp)) != NULL) {
    // format: pack-123456789012.idx = 21 characters
    if (strlen(dent->d_name) != 21) continue;
    if (strncmp(dent->d_name, "pack-", 5) ||
	strncmp(dent->d_name + 17, ".idx", 4)) continue;
    uint64_t num = (uint64_t)strtoull(dent->d_name + 5, NULL, 10);
    if (num == 0 && errno == EINVAL) continue;
    sprintf(fname, "%spack-%012lld", path.c_str(), num);

    Pack p(num, open(fname, O_RDONLY));
    m_packs.insert(std::make_pair(p.num(), p));
  }
 
  closedir(dirp);
}

void ChunkDatabase::dump_all(void) {
  cout << "[[dump begin]]" << endl;
  for (ChunkT::const_iterator it = m_chunks.begin(); it != m_chunks.end(); ++it) {
    cout << it->first << endl;
  }
  cout << "[[dump end]]" << endl;
}
