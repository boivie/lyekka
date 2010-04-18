#include <iostream>
#include <fstream>
#include <string>
#include <expat.h>
#include <fcntl.h>
#include "indexer.h"
#include "tree.h"
#include "cmd_handler.h"
#include "lyekka.h"
#include "lyekka_impl.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "file.h"

using namespace std;
using namespace Lyekka;
using namespace boost;
namespace bpo = boost::program_options;
using namespace google::protobuf::io;

struct MkTree {
  enum { ROOT, TREE, TREEREF, FILEREF, PART } state;
  pb::FileEntry* last_file_p;
  TreeBuilder tb;
};

static void handle_treeref(MkTree* self_p, const char** attr)
{
  pb::TreeRef* tr_p = self_p->tb.add_tree();
  for (int i = 0; attr[i]; i += 2) {
    const char* key_p = attr[i];
    const char* value_p = attr[i + 1];

    if (!strcmp(key_p, "name"))
      tr_p->set_name(string(value_p));
    else if (!strcmp(key_p, "mode"))
      tr_p->set_mode(strtoul(value_p, NULL, 8));
    else if (!strcmp(key_p, "mtime"))
      tr_p->set_mtime(strtoull(value_p, NULL, 10));
    else if (!strcmp(key_p, "sha")) {
      Sha sha;
      string str;
      str.resize(SHA_BITS/8);
      base16_decode((unsigned char*)str.c_str(), value_p);
      tr_p->SetExtension(pb::tree_sha_ext, str);
    } else if (!strcmp(key_p, "key")) {
      unsigned char b16[SHA_BITS/8];
      base16_decode(b16, value_p, SHA_BITS/4);
      tr_p->set_key(b16, sizeof(b16));
    } else {
      cerr << "Invalid attribute: " << key_p << endl;
      throw BadSyntaxException("Bad Input");
    }
  }
}

static void handle_fileref(MkTree* self_p, const char** attr)
{
  pb::FileEntry* fe_p = self_p->tb.add_file();
  self_p->last_file_p = fe_p;
  for (int i = 0; attr[i]; i += 2) {
    const char* key_p = attr[i];
    const char* value_p = attr[i + 1];

    if (!strcmp(key_p, "name"))
      fe_p->set_name(string(value_p));
    else if (!strcmp(key_p, "mode"))
      fe_p->set_mode(strtoul(value_p, NULL, 8));
    else if (!strcmp(key_p, "mtime"))
      fe_p->set_mtime(strtoull(value_p, NULL, 10));
    else if (!strcmp(key_p, "size"))
      fe_p->set_size(strtoull(value_p, NULL, 10));
    else {
      cerr << "Invalid attribute: " << key_p << endl;
      throw BadSyntaxException("Bad Input");
    }
  }
}

static void handle_part(MkTree* self_p, const char** attr)
{
  pb::Part* pt_p = self_p->last_file_p->add_parts();
  for (int i = 0; attr[i]; i += 2) {
    const char* key_p = attr[i];
    const char* value_p = attr[i + 1];

    if (!strcmp(key_p, "offset"))
      pt_p->set_offset(strtoull(value_p, NULL, 10));
    else if (!strcmp(key_p, "size"))
      pt_p->set_size(strtoull(value_p, NULL, 10));
    else if (!strcmp(key_p, "sha")) {
      Sha sha;
      string str;
      str.resize(SHA_BITS/8);
      base16_decode((unsigned char*)str.c_str(), value_p);
      pt_p->SetExtension(pb::part_sha_ext, str);
    } else if (!strcmp(key_p, "key")) {
      unsigned char b16[SHA_BITS/8];
      base16_decode(b16, value_p, SHA_BITS/4);
      pt_p->set_key(b16, sizeof(b16));
    } else {
      cerr << "Invalid attribute: " << key_p << endl;
      throw BadSyntaxException("Bad Input");
    }
  }
}

static void startElement(void* ud, const char *name, const char **atts)
{
  MkTree* self_p = (MkTree*)ud;
  if (strcmp(name, "tree") == 0 && self_p->state == MkTree::ROOT) {
    self_p->state = MkTree::TREE;
  } else if (strcmp(name, "treeref") == 0 && self_p->state == MkTree::TREE) {
    handle_treeref(self_p, atts);
    self_p->state = MkTree::TREEREF;
  } else if (strcmp(name, "fileref") == 0 && self_p->state == MkTree::TREE) {
    handle_fileref(self_p, atts);
    self_p->state = MkTree::FILEREF;
  } else if (strcmp(name, "part") == 0 && self_p->state == MkTree::FILEREF) {
    handle_part(self_p, atts); 
    self_p->state = MkTree::PART;
  } else {
    cerr << "Invalid token: " << name << " at state " << self_p->state << endl;
    throw BadSyntaxException("Invalid input");
  }
}

static void endElement(void *ud, const char *name)
{
  MkTree* self_p = (MkTree*)ud;
  if (self_p->state == MkTree::PART) {
    self_p->state = MkTree::FILEREF;
  } else if (self_p->state == MkTree::FILEREF) {
    self_p->state = MkTree::TREE;
  } else if (self_p->state == MkTree::TREEREF) {
    self_p->state = MkTree::TREE;
  } else if (self_p->state == MkTree::TREE) {
    self_p->state = MkTree::ROOT;
  }
}

static int mktree_cmd(CommandLineParser& c)
{
  string input;
  bool encrypt = false;
  bool done;
  char data[2048];
  MkTree self;
  XML_Parser parser = XML_ParserCreate(NULL);
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file")
    ("encrypt,e", bpo::bool_switch(&encrypt), "encrypt");
  c.p.add("input", 1);
  c.parse_options();

  self.state = MkTree::ROOT;
  XML_SetUserData(parser, &self);
  XML_SetElementHandler(parser, startElement, endElement);

  fstream fis(input.c_str(), ios_base::in);
  do {
    fis.read(data, sizeof(data));
    done = fis.gcount() < sizeof(data);
    if (!XML_Parse(parser, data, fis.gcount(), done)) {
      cerr << "Failed to parse data: " 
	   << XML_ErrorString(XML_GetErrorCode(parser)) << endl;
      return 1;
    }
  } while (!done);
  fis.close();
  XML_ParserFree(parser);

  FileWriter fw;
  auto_ptr<const Tree> tree_p = self.tb.build();
  auto_ptr<ObjectIdentifier> oi_p = tree_p->serialize(&fw.get_writer(), encrypt);
  char base16[256/4 + 1];
  if (encrypt) {
    cout << oi_p->sha().base16(base16) 
	 << " " << oi_p->key().base16() << endl;
  } else {
    cout << oi_p->sha().base16(base16) << endl;
  }
  fw.commit(oi_p->sha());
  return 0;
}

LYEKKA_COMMAND(mktree_cmd, "mktree", "", "Creates a tree object from formatted input");

