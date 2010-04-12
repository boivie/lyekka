#ifndef OBJECT_GENERATOR_H_INCLUSION_GUARD
#define OBJECT_GENERATOR_H_INCLUSION_GUARD

#include "tree.h"
#include "blob.h"
#include "object.h"
#include "faf.h"

namespace Lyekka {


class ObjectGenerator {
public:
  ObjectGenerator(ObjectWriter& dest)
    : m_dest(dest) {}
  virtual ~ObjectGenerator() {}
  Sha generate(FolderPtr src_p);
  virtual void on_folder(const Tree& tree) = 0;
  virtual void on_blob(const Blob& blob) = 0;
private:
  ObjectWriter& m_dest;
};


}

#endif
