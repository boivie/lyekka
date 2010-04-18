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

static int show_object_cmd(CommandLineParser& c)
{
  string input = "";
  bool type = false;
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file")
    ("type,t", bpo::bool_switch(&type), "show type");
  c.p.add("input", -1);
  c.parse_options();
  FileInputStream fis(xopen(input.c_str(), O_RDONLY));
  fis.SetCloseOnDelete(true);

  if (type) {
    const void* mem_p;
    int size;
    
    if (!fis.Next(&mem_p, &size) ||
	size < 4) {
      cerr << "Failed to read file" << endl;
      return 1;
    }
    if (memcmp(mem_p, "blob", 4) == 0) {
      cout << "blob" << endl;
    } else if (memcmp(mem_p, "tree", 4) == 0) {
      cout << "tree" << endl;
    } else if (memcmp(mem_p, "TREE", 4) == 0) {
      cout << "encrypted tree" << endl;
    } else if (memcmp(mem_p, "mfst", 4) == 0) {
      cout << "manifest" << endl;
    } else {
      cerr << "invalid object magic - probably not a valid object." << endl;
      return 1;
    }
    return 0;
  }
  return 0;
}

LYEKKA_COMMAND(show_object_cmd, "show-object", "", "Shows objects");

