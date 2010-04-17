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
  bool encrypt = false;
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file")
    ("offset", bpo::value<uint64_t>(&offset), "offset")
    ("length", bpo::value<uint32_t>(&length), "length")
    ("encrypt,e", bpo::bool_switch(&encrypt), "encrypt");
  c.p.add("input", 1);
  c.parse_options();
  int in_fd = open(input.c_str(), O_RDONLY);
  FileWriter fw;
 
  if (in_fd == -1) {
    cerr << "Failed to open file: " << strerror(errno);
    return 1;
  }

  MmapInputStream mmis(in_fd, offset, length);
  if (encrypt) {
    int size = mmis.ByteCount();
    cerr << "Generating key" << endl;
    boost::shared_ptr<AesKey> key_p = Blob::generate_key(&mmis);
    cerr << "Generating blob " << size << endl;
    mmis.BackUp(size - mmis.ByteCount());
    Blob blob = Blob::create(&mmis, &fw.get_writer(), key_p.get());
    fw.commit(blob.hash());
    char sha_buf[256/4 + 1];
    char key_buf[128/4 + 1];
    char iv_buf[128/4 + 1];
    cout << blob.hash().base16(sha_buf) << " " 
	 << base16_encode(key_buf, key_p->key(), 128 / 8) 
	 << base16_encode(iv_buf, key_p->iv(), 128 / 8) << endl;
  } else {
    Blob blob = Blob::create(&mmis, &fw.get_writer(), NULL);
    fw.commit(blob.hash());
    char buf[256/4 + 1];
    cout << blob.hash().base16(buf) << endl;
  }
  return 0;
}

static int cat_blob(CommandLineParser& c)
{
  string input;
  string key = "";
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file")
    ("key,K", bpo::value<string>(&key), "key");
  c.p.add("input", 1);
  c.parse_options();
  FileReader fr;
  Sha sha;
  sha.set_base16(input.c_str());
  boost::shared_ptr<ObjectInputStream> is_p = fr.find(sha);
  FileOutputStream fos(STDOUT_FILENO);

  if (key == "") {
    Blob::unpack(is_p.get(), &fos, NULL);
  } else {
    uint8_t key_buf[128/8];
    uint8_t iv_buf[128/8];
    Aes128Key akey(base16_decode(key_buf, key.c_str(), 128/4),
		   base16_decode(iv_buf, key.c_str() + 128/4, 128/4));
    Blob::unpack(is_p.get(), &fos, &akey);
  }
  return 0;
}

static int strip_blob(CommandLineParser& c)
{
  const void* mem_p;
  int size;
  string input;
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file");
  c.p.add("input", 1);
  c.parse_options();
  FileInputStream fis(open(input.c_str(), O_RDONLY));
  fis.SetCloseOnDelete(true);

  if (!fis.Next(&mem_p, &size) ||
      size < 4 ||
      memcmp(mem_p, "blob", 4) != 0) {
    cerr << "Invalid blob header" << endl;
    return 1;
  }

  fis.BackUp(size - 4);
  FileOutputStream fos(STDOUT_FILENO);
  copy_streams(&fos, &fis);
  return 0;
}

LYEKKA_COMMAND(create_blob, "create-blob", "-i <input_file> -o <output_file>", "Creates a blob");
LYEKKA_COMMAND(cat_blob, "cat-blob", "-i <input_file>", "Extracts a blob");
LYEKKA_COMMAND(strip_blob, "strip-blob", "-i <input_file>", "Strips a blob from its header");
