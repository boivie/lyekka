#include <iostream>
#include <fstream>
#include <fcntl.h>
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "cmd_handler.h"
#include "lyekka.h"
#include "blob.h"
#include <sys/stat.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <errno.h>
#include <cstdio>
#include "file.h"

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;
namespace fs = boost::filesystem;
namespace bpo = boost::program_options;

static int create_blob(CommandLineParser& c)
{
  string input;
  string output;
  uint64_t offset = 0;
  uint32_t length = 0xFFFFFFFF;
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file")
    ("output,o", bpo::value<string>(&output), "output file")
    ("offset", bpo::value<uint64_t>(&offset), "offset")
    ("length", bpo::value<uint32_t>(&length), "length");
  c.parse_options();
  FileReader fr;
  Sha sha;
  sha.set_base16(input.c_str());
  boost::shared_ptr<ObjectInputStream> is_p = fr.find(sha);
  int out_fd = open(output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0755);
  
  if (out_fd == -1) {
    cerr << "Failed to open file: " << strerror(errno);
    return 1;
  }

  if (offset > 0)
    is_p->Skip(offset);

  FileOutputStream fos(out_fd);
  fos.SetCloseOnDelete(true);
  Blob blob = Blob::create(is_p.get(), &fos);
  char buf[256/4 + 1];
  cout << blob.hash().base16(buf) << endl;
  return 0;
}

static int cat_blob(CommandLineParser& c)
{
  string input;
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file");
  c.parse_options();
  FileReader fr;
  Sha sha;
  sha.set_base16(input.c_str());
  boost::shared_ptr<ObjectInputStream> is_p = fr.find(sha);
  FileOutputStream fos(STDOUT_FILENO);

  Blob::unpack(is_p.get(), &fos);
  return 0;
}


LYEKKA_COMMAND(create_blob, "create-blob", "-i <input_file> -o <output_file>", "Creates a blob");
LYEKKA_COMMAND(cat_blob, "cat-blob", "-i <input_file>", "Extracts a blob");
