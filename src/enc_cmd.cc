#include <fcntl.h>
#include <iostream>
#include <string>
#include "cmd_handler.h"
#include "aes_stream.h"
#include "lyekka.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;
namespace bpo = boost::program_options;

static int enc_cmd(CommandLineParser& c)
{
  string in, out, key, iv, cipher;
  bool decrypt = false;
  c.po.add_options()
    ("cipher,c", bpo::value<string>(&cipher), "cipher")
    ("key,K", bpo::value<string>(&key), "key")
    ("iv", bpo::value<string>(&iv), "iv")
    ("in", bpo::value<string>(&in), "input file")
    ("out", bpo::value<string>(&out), "output file")
    ("decrypt,d", bpo::bool_switch(&decrypt), "decrypt");
  c.parse_options();

  if (cipher != "aes-128-cbc") {
    cerr << "Invalid cipher: " << cipher << endl;
    return 1;
  }

  if (key.length() != 128/4 || iv.length() != 128/4) {
    cerr << "Invalid key or IV length" << endl;
    return 1;
  }

  uint8_t key_buf[128/8];
  uint8_t iv_buf[128/8];
  base16_decode(key_buf, key.c_str());
  base16_decode(iv_buf, iv.c_str());

  if (!decrypt) {
    FileInputStream fis(open(in.c_str(), O_RDONLY));
    FileOutputStream fos(open(out.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644));
    AesOutputStream aos(&fos, AesOutputStream::CBC, Aes128Key(key_buf, iv_buf));
    copy_streams(&aos, &fis);
  } else {
    FileInputStream fis(open(in.c_str(), O_RDONLY));
    FileOutputStream fos(open(out.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644));
    AesInputStream ais(&fis, AesInputStream::CBC, Aes128Key(key_buf, iv_buf));
    copy_streams(&fos, &ais);
  }
  return 0;
}

LYEKKA_COMMAND(enc_cmd, "enc", "", "Encrypts or decrypts a file");

