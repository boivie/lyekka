#include <fcntl.h>
#include "lyekka.h"
#include "object_generator.h"
#include "lyekka_impl.pb.h"
#include "blob.h"
#include "mmap_stream.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace google::protobuf::io;
using namespace Lyekka;
using namespace boost;
using namespace std;

auto_ptr<ObjectIdentifier> ObjectGenerator::generate(FolderPtr f, bool encrypt)
{
  TreeBuilder tb;
  FolderList folders = f->get_sub_folders();
  for (FolderList::iterator child_i = folders.begin(); 
       child_i != folders.end();
       ++child_i) {
    pb::TreeRef* tr_p = tb.add_tree();
    tr_p->MergeFrom((*child_i)->pb());
    auto_ptr<ObjectIdentifier> child_oi_p = generate(*child_i, encrypt);
    tr_p->SetExtension(pb::tree_sha_ext, child_oi_p->sha().mutable_string());
    if (child_oi_p->has_key()) {
      char keybuf[256/8];
      memcpy(keybuf, child_oi_p->key().key(), 128/8);
      memcpy(keybuf + 128/8, child_oi_p->key().iv(), 128/8);
      tr_p->set_key(keybuf, 256/8);
    }
  }

  FileList files = f->get_files();
  for (FileList::iterator child_i = files.begin();
       child_i != files.end();
       ++child_i) {
    FilePtr file_p = *child_i;
    pb::FileEntry* fe_p = tb.add_file();
    fe_p->MergeFrom(file_p->pb());

    int in_fd = xopen(file_p->path().string().c_str(), O_RDONLY);
    PartList parts = file_p->get_parts();
    for (PartList::iterator part_i = parts.begin();
	 part_i != parts.end();
	 ++part_i) {
      PartPtr part_p = *part_i;
      ZeroCopyOutputStream& os = m_dest.get_writer();
      MmapInputStream mmis(in_fd, part_p->offset(), part_p->size());
      auto_ptr<ObjectIdentifier> part_oi_p;
      pb::Part* pt_p = fe_p->add_parts();
      if (encrypt) {
	int size = mmis.ByteCount();
	auto_ptr<AesKey> key_p = Blob::generate_key(&mmis);
	mmis.BackUp(size - mmis.ByteCount());
	part_oi_p = Blob::create(&mmis, &os, key_p.get());
	char keybuf[256/8];
	memcpy(keybuf, key_p->key(), 128/8);
	memcpy(keybuf + 128/8, key_p->iv(), 128/8);
	pt_p->set_key(keybuf, 256/8);
      } else {
	part_oi_p = Blob::create(&mmis, &os);
      }
      on_blob(*part_oi_p);
      pt_p->set_offset(part_p->offset());
      pt_p->set_size(part_p->size());
      pt_p->SetExtension(pb::part_sha_ext, part_oi_p->sha().mutable_string());
      m_dest.commit(part_oi_p->sha());
    }
    close(in_fd);
  }

  auto_ptr<const Tree> tree_p = tb.build();
  ZeroCopyOutputStream& os = m_dest.get_writer();
  auto_ptr<ObjectIdentifier> oi_p = tree_p->serialize(&os, encrypt);
  on_folder(*oi_p, *tree_p);
  m_dest.commit(oi_p->sha());
  return oi_p;
}
