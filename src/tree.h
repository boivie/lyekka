#ifndef TREE_H_INCLUSION_GUARD
#define TREE_H_INCLUSION_GUARD

#include <string>
#include <list>
#include <vector>
#include <iostream>
#include "sha.h"
#include "lyekka.pb.h"
#include <boost/shared_ptr.hpp>
#include "aes_stream.h"
#include "object.h"

namespace Lyekka {

class InvalidTreeException
{
public:
  InvalidTreeException(const std::string extra = "") : m_extra(extra) {};
  const char* what() { return m_extra.c_str(); }
private:
  const std::string m_extra;
};

class TreeBuilder;

class Tree {
public:
  friend class TreeBuilder;
  std::auto_ptr<ObjectIdentifier> serialize(google::protobuf::io::ZeroCopyOutputStream* os_p, 
					    bool encrypt = false) const;
  static std::auto_ptr<const Tree> deserialize(google::protobuf::io::ZeroCopyInputStream* is_p, const AesKey* key_p = NULL);
  const std::vector<Sha>& get_refs() const { return m_refs; }
  const Sha& get_ref(int idx) const { return m_refs[idx]; }
  const Lyekka::pb::Tree& get_pb() const { return m_pb; }
private:
  Lyekka::pb::Tree m_pb;
  std::vector<Sha> m_refs;
};


class TreeBuilder {
public:
  TreeBuilder(void) : m_tree(new Tree()) {}
  static bool compare_treeref (const pb::TreeRef& first_p, const pb::TreeRef& second_p);
  static bool compare_fileentry (const pb::FileEntry& first, const pb::FileEntry& second);
  Lyekka::pb::TreeRef* add_tree() { return m_tree->m_pb.add_subdirs(); }
  Lyekka::pb::FileEntry* add_file() { return m_tree->m_pb.add_files(); }
  std::auto_ptr<const Tree> build();
private:
  int find_idx(const std::string& str);
  std::auto_ptr<Tree> m_tree;
};



}

#endif
