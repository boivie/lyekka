#include <fcntl.h>
#include <iostream>
#include <string>
#include "cmd_handler.h"
#include "lyekka.h"
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;
namespace bpo = boost::program_options;

static int zlib_cmd(CommandLineParser& c)
{
  string in, out;
  bool decompress = false;
  c.po.add_options()
    ("decompress,d", bpo::bool_switch(&decompress), "decompress");    
  c.parse_options();

  if (decompress) {
    FileInputStream fis(STDIN_FILENO);
    GzipInputStream gzis(&fis, GzipInputStream::ZLIB);
    FileOutputStream fos(STDOUT_FILENO);
    copy_streams(&fos, &gzis);
  } else {
    GzipOutputStream::Options o;
    o.format = GzipOutputStream::ZLIB;
    FileInputStream fis(STDIN_FILENO);
    FileOutputStream fos(STDOUT_FILENO);
    GzipOutputStream gzos(&fos, o);
    copy_streams(&gzos, &fis);
  }
  return 0;
}

LYEKKA_COMMAND(zlib_cmd, "zlib", "", "Compresses or decompresses a file");

