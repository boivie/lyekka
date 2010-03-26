#ifndef OBJ_REF_H_INCLUSION_GUARD
#define OBJ_REF_H_INCLUSION_GUARD

#include "sha.h"

namespace Lyekka {

class ObjRef {
public:
  ObjRef() {}
  ObjRef(const Sha& sha) : m_sha(sha) { }
  ObjRef(const Sha& sha, const Sha& key) : m_sha(sha), m_key(key) { }
  
  const Sha& sha() const { return m_sha; }
  const Sha& key() const { return m_key; }
  bool has_key() const { return m_has_key; }
	
private:
  Sha m_key;
  Sha m_sha;
  bool m_has_key;
};

}

#endif
