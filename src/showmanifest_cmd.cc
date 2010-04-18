#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include "manifest.h"
#include "lyekka.h"
#include "cmd_handler.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;
using namespace Lyekka;
using namespace boost;
namespace bpo = boost::program_options;
using namespace google::protobuf::io;

static void show_full(const Manifest& manifest)
{
  char b16[SHA_BITS/4 + 1];
  char fileb16[SHA_BITS/4 + 1];
  
  cout << "Entry-Point: " << manifest.entry_point().base16(b16) << endl
       << "Properties:" << endl;
  for (int i = 0; i < manifest.pb().properties().properties_size(); i++) {
    const pb::Property& pt = manifest.pb().properties().properties(i);
    cout << "  " << pt.key() << ": " << pt.str_val() << endl;
  }
  cout << "Secure Properties:" << endl;
  for (int i = 0; i < manifest.secure_properties().properties_size(); i++) {
    const pb::Property& pt = manifest.secure_properties().properties(i);
    cout << "  " << pt.key() << ": " << pt.str_val() << endl;
  }
}

static int show_manifest(CommandLineParser& c)
{
  string input;
  string key = "";
  string show = "full";
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file")
    ("show,s", bpo::value<string>(&show), "what to show")
    ("key,K", bpo::value<string>(&key), "key");
  c.p.add("input", 1);
  c.parse_options();

  try {
    FileInputStream fis(xopen(input.c_str(), O_RDONLY));
    fis.SetCloseOnDelete(true);
    
    auto_ptr<Manifest> manifest_p = Manifest::deserialize(&fis);
    if (key != "") {
      key = key + key;
      auto_ptr<AesKey> key_p = AesKey::create(key);
      manifest_p->decrypt(*key_p);
    }

    if (show == "full")
      show_full(*manifest_p);
    //    else if (show == "raw")
    //      show_raw(*manifest_p);
    return 0;
  } catch (InvalidManifestException& ex) {
    cerr << "Failed to parse manifest: " << ex.what() << endl;
    return 1;
  }
}

LYEKKA_LL_COMMAND(show_manifest, "show-manifest", "", "Lists a manifest's contents");

