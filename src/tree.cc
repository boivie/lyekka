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

bool TreeBuilder::compare_fileentry (const pb::FileEntry& first, const pb::FileEntry& second)
{
  return strcmp(first.name().c_str(), second.name().c_str()) < 0;
}

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

auto_ptr<Tree> TreeBuilder::build(void)
{
  // Files and subdirs are sorted by name
  std::sort(m_tree->m_pb.mutable_subdirs()->begin(),
	    m_tree->m_pb.mutable_subdirs()->end(),
	    compare_treeref); 

  std::sort(m_tree->m_pb.mutable_files()->begin(),
	    m_tree->m_pb.mutable_files()->end(),
	    compare_fileentry); 

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
  auto_ptr<Tree> old_p = m_tree;
  m_tree.reset(new Tree());
  return old_p;
}

void Tree::create_key() 
{
  FileOutputStream fos(open("/dev/null", O_WRONLY));
  fos.SetCloseOnDelete(true);
  Sha256OutputStream hos(&fos);
  m_pb.SerializeToZeroCopyStream(&hos);
  const Sha sha = hos.get_digest();
  m_key.set_key(sha.data());
  m_key.set_iv(sha.data() + SHA_BITS/8/2);
}

void Tree::serialize(ZeroCopyOutputStream* os_p, bool encrypt)
{
  Sha256OutputStream hos(os_p);
  write_to_stream(&hos, encrypt ? "TREE" : "tree", 4);
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
  // The protocol buffer is gzip-compressed and optionally encrypted.
  GzipOutputStream::Options o;
  o.format = GzipOutputStream::ZLIB;
  if (encrypt) {
    create_key();
    AesOutputStream aos(&hos, AesOutputStream::CBC, m_key);
    GzipOutputStream gzos(&aos, o);
    m_pb.SerializeToZeroCopyStream(&gzos);
    gzos.Close();
    aos.Close();
  } else {
    GzipOutputStream gzos(&hos, o);
    m_pb.SerializeToZeroCopyStream(&gzos);
    gzos.Close();
  }

  m_sha = hos.get_digest();
}

auto_ptr<Tree> Tree::deserialize(google::protobuf::io::ZeroCopyInputStream* is_p, 
				 const AesKey* key_p)
{
  auto_ptr<Tree> tree_p(new Tree());
  int32_t pb_size = -1;
  bool is_encrypted = false;
  {
    CodedInputStream cis(is_p);
    char header[4];
    cis.ReadRaw(&header, sizeof(header));

    if (memcmp(header, "tree", 4) == 0) {
    } else if (memcmp(header, "TREE", 4) == 0) {
      is_encrypted = true;
    } else {
      throw InvalidTreeException("Invalid magic");
    }
    
    int32_t refs_size = -1;
    cis.ReadRaw(&refs_size, sizeof(refs_size));
    refs_size = ntohl(refs_size);
    if (refs_size < 0 || (refs_size % (SHA_BITS/8)) != 0)
      throw InvalidTreeException("Invalid refs size");
    
    tree_p->m_refs.resize(refs_size / (SHA_BITS/8));
    for (int i = 0; i < (refs_size / (SHA_BITS/8)); i++) {
      cis.ReadRaw(tree_p->m_refs[i].mutable_data(), SHA_BITS/8);
    }
    
    cis.ReadRaw(&pb_size, sizeof(pb_size));
    pb_size = ntohl(pb_size);
    if (pb_size < 0)
      throw InvalidTreeException("Invalid pb size");
  }
  
  if (!is_encrypted) {
    GzipInputStream gzis(is_p, GzipInputStream::ZLIB);
    if (!tree_p->m_pb.ParseFromZeroCopyStream(&gzis))
      throw InvalidTreeException("Invalid protocol buffer");
  } else if (is_encrypted && key_p != NULL) {
    AesInputStream ais(is_p, AesInputStream::CBC, *key_p);
    GzipInputStream gzis(&ais, GzipInputStream::ZLIB);
    if (!tree_p->m_pb.ParseFromZeroCopyStream(&gzis))
      throw InvalidTreeException("Invalid protocol buffer");
  }
  return tree_p;
}

auto_ptr<AesKey> Tree::get_key(void) const 
{
  return auto_ptr<AesKey>(new Aes128Key(m_key.key(), m_key.iv()));
}
