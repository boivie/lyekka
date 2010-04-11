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
#include "mmap_stream.h"

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
  uint32_t length = 0;
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file")
    ("offset", bpo::value<uint64_t>(&offset), "offset")
    ("length", bpo::value<uint32_t>(&length), "length");
  c.p.add("input", 1);
  c.parse_options();
  int in_fd = open(input.c_str(), O_RDONLY);
  FileWriter fw;
 
  if (in_fd == -1) {
    cerr << "Failed to open file: " << strerror(errno);
    return 1;
  }

  MmapInputStream mmis(in_fd, offset, length);
  Blob blob = Blob::create(&mmis, &fw.get_writer());
  fw.commit(blob.hash());
  char buf[256/4 + 1];
  cout << blob.hash().base16(buf) << endl;
  return 0;
}

static int cat_blob(CommandLineParser& c)
{
  string input;
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file");
  c.p.add("input", 1);
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
