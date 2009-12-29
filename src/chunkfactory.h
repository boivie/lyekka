#ifndef CHUNKFACTORY_H_INCLUSION_GUARD
#define CHUNKFACTORY_H_INCLUSION_GUARD

#include "chunk.h"
#include <boost/filesystem/path.hpp>
#include <iostream>

namespace Lyekka {
  class ChunkFactory {
  public: 
    static Chunk create(boost::filesystem::path& path, uint64_t offset, size_t size, std::ostream& out);
    
  };
} 

#endif
