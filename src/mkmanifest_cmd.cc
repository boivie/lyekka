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
#include "manifest.h"

using namespace std;
using namespace Lyekka;
using namespace boost;
namespace bpo = boost::program_options;
using namespace google::protobuf::io;

struct MkManifest {
  enum { ROOT, MANIFEST, ENTRY_POINT, 
	 PROPERTIES, PROPERTIES_P, 
	 SECURE_PROPERTIES, SECURE_PROPERTIES_P } state;
  std::string name;
  std::string value;
  Manifest mfst;
};

static void characterHandler(void *ud, const XML_Char *s, int len)
{
  MkManifest* self_p = (MkManifest*)ud;
  self_p->value = string(s, len);
}

static void handle_property(MkManifest* self_p, const char **attr)
{
  for (int i = 0; attr[i]; i += 2) {
    const char* key_p = attr[i];
    const char* value_p = attr[i + 1];

    if (!strcmp(key_p, "name")) {
      self_p->name = string(value_p);
    } else {
      cerr << "Invalid attribute: " << key_p << endl;
      throw BadSyntaxException("Bad Input");
    }
  }
}

static void startElement(void* ud, const char *name, const char **atts)
{
  MkManifest* self_p = (MkManifest*)ud;
  if (strcmp(name, "manifest") == 0 && 
      self_p->state == MkManifest::ROOT) {
    self_p->state = MkManifest::MANIFEST;
  } else if (strcmp(name, "entry_point") == 0 && 
	     self_p->state == MkManifest::MANIFEST) {
    self_p->state = MkManifest::ENTRY_POINT;
  } else if (strcmp(name, "properties") == 0 && 
	     self_p->state == MkManifest::MANIFEST) {
    self_p->state = MkManifest::PROPERTIES;
  } else if (strcmp(name, "secure_properties") == 0 && 
	     self_p->state == MkManifest::MANIFEST) {
    self_p->state = MkManifest::SECURE_PROPERTIES;
  } else if (strcmp(name, "property") == 0 && 
	     self_p->state == MkManifest::PROPERTIES) {
    handle_property(self_p, atts);
    self_p->state = MkManifest::PROPERTIES_P;
  } else if (strcmp(name, "property") == 0 && 
	     self_p->state == MkManifest::SECURE_PROPERTIES) {
    handle_property(self_p, atts);
    self_p->state = MkManifest::SECURE_PROPERTIES_P;
  } else {
    cerr << "Invalid token: " << name << " at state " << self_p->state << endl;
    throw BadSyntaxException("Invalid input");
  }
}

static void endElement(void *ud, const char *name)
{
  MkManifest* self_p = (MkManifest*)ud;
  if (self_p->state == MkManifest::PROPERTIES_P) {
    self_p->mfst.add_property(self_p->name, self_p->value);
    self_p->state = MkManifest::PROPERTIES;
  } else if (self_p->state == MkManifest::SECURE_PROPERTIES_P) {
    self_p->mfst.add_secure_property(self_p->name, self_p->value);
    self_p->state = MkManifest::SECURE_PROPERTIES;
  } else if (self_p->state == MkManifest::PROPERTIES) {
    self_p->state = MkManifest::MANIFEST;
  } else if (self_p->state == MkManifest::SECURE_PROPERTIES) {
    self_p->state = MkManifest::MANIFEST;
  } else if (self_p->state == MkManifest::ENTRY_POINT) {
    Sha sha;
    sha.set_base16(self_p->value.c_str());
    self_p->mfst.set_entry_point(sha);
    self_p->state = MkManifest::MANIFEST;
  } else if (self_p->state == MkManifest::MANIFEST) {
    self_p->state = MkManifest::ROOT;
  }
}

static int mkmanifest_cmd(CommandLineParser& c)
{
  string input;
  bool done;
  char data[2048];
  MkManifest self;
  XML_Parser parser = XML_ParserCreate(NULL);
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file");
  c.p.add("input", 1);
  c.parse_options();

  self.state = MkManifest::ROOT;
  XML_SetUserData(parser, &self);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, characterHandler);

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
  
  auto_ptr<ObjectIdentifier> oi_p = self.mfst.serialize(&fw.get_writer());
  char base16[256/4 + 1];
  char keybuf[128/4 + 1];
  cout << oi_p->sha().base16(base16) << " " 
       << base16_encode(keybuf, oi_p->key().key(), 128/8) << endl;
  fw.commit(oi_p->sha());
  return 0;
}

LYEKKA_COMMAND(mkmanifest_cmd, "mk-manifest", "", "Creates a manifest object from formatted input");

