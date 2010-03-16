#include <iostream>
#include <fstream>
#include <fcntl.h>
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "cmd_handler.h"
#include "lyekka.h"
#include "blob.h"
#include <sys/stat.h>

using namespace std;
using namespace Lyekka;
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
  int in_fd = open(input.c_str(), O_RDONLY);
  int out_fd = open(output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0755);
  if (in_fd == -1 || out_fd == -1) {
    cerr << "Failed to open file: " << strerror(errno);
    return 1;
  }
  struct stat st;
  if (fstat(in_fd, &st) == -1) {
    cerr << "Failed to stat input: " << strerror(errno);
    return -1;
  }

  if (offset > st.st_size) {
    cerr << "Invalid offset" << endl;
    return 1;
  }

  if (length > (st.st_size - offset))
    length = st.st_size - offset;

  Blob blob = Blob::create_from_fd(in_fd, offset, length, out_fd);
  char buf[256/4 + 1];
  cout << sha256_base16(blob.hash, buf) << endl;
  return 0;
}

static int cat_blob(CommandLineParser& c)
{
  string input;
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file");
  c.parse_options();
  int in_fd = open(input.c_str(), O_RDONLY);

  if (in_fd == -1) {
    cerr << "Failed to open file: " << strerror(errno);
    return 1;
  }
  struct stat st;
  if (fstat(in_fd, &st) == -1) {
    cerr << "Failed to stat input: " << strerror(errno);
    return -1;
  }

  Blob::unpack_from_fd(in_fd, st.st_size, STDOUT_FILENO);
  return 0;
}


LYEKKA_COMMAND(create_blob, "create-blob", "-i <input_file> -o <output_file>", "Creates a blob");
LYEKKA_COMMAND(cat_blob, "cat-blob", "-i <input_file>", "Extracts a blob");