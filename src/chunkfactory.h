#ifndef CHUNKFACTORY_H_INCLUSION_GUARD
#define CHUNKFACTORY_H_INCLUSION_GUARD

#include "chunk.h"
#include <boost/filesystem/path.hpp>

namespace Lyekka {

  class ChunkBuffer {
  public:
    
  };

  class ChunkFactory {
  public: 
    static Chunk create(boost::filesystem::path& path, uint64_t offset, size_t size, ChunkBuffer& buffer);
    
  };
} 

#endif
