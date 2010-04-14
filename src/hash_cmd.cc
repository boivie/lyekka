#include <fcntl.h>
#include <iostream>
#include <string>
#include "cmd_handler.h"
#include "hash_stream.h"
#include "lyekka.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;
namespace bpo = boost::program_options;

static int sha256(CommandLineParser& c)
{
  string input = "";
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file");
  c.p.add("input", -1);
  c.parse_options();
  auto_ptr<FileInputStream> fis_p;

  if (input != "") {
    fis_p.reset(new FileInputStream(open(input.c_str(), O_RDONLY)));
  } else {
    fis_p.reset(new FileInputStream(STDIN_FILENO));
  }
  FileOutputStream fos(open("/dev/null", O_WRONLY));
  Sha256OutputStream hos(&fos);

  copy_streams(&hos, fis_p.get());
  
  char base16[256/4+1];
  cout << hos.get_digest().base16(base16) << endl;

  return 0;
}

LYEKKA_COMMAND(sha256, "sha256", "", "Hashes a file");

