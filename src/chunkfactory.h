#ifndef CHUNKFACTORY_H_INCLUSION_GUARD
#define CHUNKFACTORY_H_INCLUSION_GUARD

#include "chunk.h"
#include <iostream>

namespace Lyekka {
  class ChunkFactory {
  public: 
    static ChunkHash generate_hash(std::istream& in, uint64_t offset, size_t size);
    static Chunk generate_chunk(std::istream& in, uint64_t offset, size_t size, const ChunkHash& key, std::ostream& out);
  };
} 

#endif
