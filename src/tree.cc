#include <arpa/inet.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "tree.h"
#include "hash_stream.h"
#include <fcntl.h>
#include "lyekka_impl.pb.h"
#include "lyekka.h"
#include <google/protobuf/io/gzip_stream.h>

using namespace Lyekka;
using namespace std;
using namespace boost;
using namespace google::protobuf::io;

bool TreeBuilder::compare_treeref (const pb::TreeRef& first, const pb::TreeRef& second)
{
  return strcmp(first.name().c_str(), second.name().c_str()) < 0;
}

int TreeBuilder::find_idx(const string& str)
{
  Sha sha;
  memcpy(sha.mutable_data(), str.c_str(), SHA_BITS/8);
  
  return lower_bound(m_tree->m_refs.begin(),
		     m_tree->m_refs.end(),
		     sha) - m_tree->m_refs.begin();
}

shared_ptr<Tree> TreeBuilder::build(void)
{
  // Files and subdirs are sorted by name
  std::sort(m_tree->m_pb.mutable_subdirs()->begin(),
	    m_tree->m_pb.mutable_subdirs()->end(),
	    compare_treeref); 

  // The refs table have to be contain all SHAs, sorted by value.
  for (int i = 0; i < m_tree->m_pb.subdirs_size(); i++) {
    const pb::TreeRef& tr = m_tree->m_pb.subdirs(i);
    Sha sha;
    const string& str = tr.GetExtension(pb::tree_sha_ext);
    memcpy(sha.mutable_data(), str.c_str(), SHA_BITS/8);
    m_tree->m_refs.push_back(sha);
  }
  for (int i = 0; i < m_tree->m_pb.files_size(); i++) {
    const pb::FileEntry& fe = m_tree->m_pb.files(i);
    for (int j = 0; j < fe.parts_size(); j++) {
      const pb::Part& pt = fe.parts(j);
      Sha sha;
      const string& str = pt.GetExtension(pb::part_sha_ext);
      memcpy(sha.mutable_data(), str.c_str(), SHA_BITS/8);
      m_tree->m_refs.push_back(sha);
    }
  }
  std::sort(m_tree->m_refs.begin(), m_tree->m_refs.end());
  m_tree->m_refs.erase(std::unique(m_tree->m_refs.begin(), m_tree->m_refs.end()), m_tree->m_refs.end());

  // And the trees and parts shouldn't contain the SHAs, just indexes into the refs table.
  for (int i = 0; i < m_tree->m_pb.subdirs_size(); i++) {
    pb::TreeRef* tr_p = m_tree->m_pb.mutable_subdirs(i);
    string shav = tr_p->GetExtension(pb::tree_sha_ext);
    tr_p->set_sha_idx(find_idx(shav));
    tr_p->ClearExtension(pb::tree_sha_ext);
  }
  for (int i = 0; i < m_tree->m_pb.files_size(); i++) {
    pb::FileEntry* fe_p = m_tree->m_pb.mutable_files(i);
    for (int j = 0; j < fe_p->parts_size(); j++) {
      pb::Part* pt_p = fe_p->mutable_parts(j);
      string shav = pt_p->GetExtension(pb::part_sha_ext);
      pt_p->set_sha_idx(find_idx(shav));
      pt_p->ClearExtension(pb::part_sha_ext);
    }
  }

  // Return the 'validated' tree and clear our instance data
  shared_ptr<Tree> old_p = m_tree;
  m_tree.reset(new Tree());
  return old_p;
}

void Tree::serialize(ZeroCopyOutputStream* os_p)
{
  // Write to stream.
  Sha256OutputStream hos(os_p);
  write_to_stream(&hos, "tree", 4);
  
  {
    CodedOutputStream cos(&hos);
    
    uint32_t refs_size = htonl(m_refs.size() * SHA_BITS/8);
    cos.WriteRaw(&refs_size, sizeof(refs_size));
    
    for (int i = 0; i < m_refs.size(); i++) {
      cos.WriteRaw(m_refs[i].data(), SHA_BITS / 8);
    }

    uint32_t pb_size = htonl(m_pb.ByteSize());
    cos.WriteRaw(&pb_size, sizeof(pb_size));
  }
  // The protocol buffer is gzip-compressed
  GzipOutputStream::Options o;
  o.format = GzipOutputStream::ZLIB;
  GzipOutputStream gzos(&hos, o);
  m_pb.SerializeToZeroCopyStream(&gzos);
  gzos.Close();

  m_sha = hos.get_digest();
}

void Tree::deserialize(google::protobuf::io::ZeroCopyInputStream* is_p)
{
  int32_t pb_size = -1;
  {
    CodedInputStream cis(is_p);
    char header[4];
    cis.ReadRaw(&header, sizeof(header));
    if (memcmp(header, "tree", 4) != 0) 
      throw InvalidTreeException("Invalid magic");
    
    int32_t refs_size = -1;
    cis.ReadRaw(&refs_size, sizeof(refs_size));
    refs_size = ntohl(refs_size);
    if (refs_size < 0 || (refs_size % (SHA_BITS/8)) != 0)
      throw InvalidTreeException("Invalid refs size");
    
    m_refs.resize(refs_size / (SHA_BITS/8));
    for (int i = 0; i < (refs_size / (SHA_BITS/8)); i++) {
      cis.ReadRaw(m_refs[i].mutable_data(), SHA_BITS/8);
    }
    
    cis.ReadRaw(&pb_size, sizeof(pb_size));
    pb_size = ntohl(pb_size);
    if (pb_size < 0)
      throw InvalidTreeException("Invalid pb size");
  }
  
  GzipInputStream gzis(is_p);
  if (!m_pb.ParseFromZeroCopyStream(&gzis))
    throw InvalidTreeException("Invalid protocol buffer");
}
