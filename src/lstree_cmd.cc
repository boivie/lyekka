#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <vector>
#include "tree.h"
#include "cmd_handler.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;
using namespace Lyekka;
using namespace boost;
namespace bpo = boost::program_options;
using namespace google::protobuf::io;

const char* mode_str(int mode) {
  static char buf[11];
  buf[0] = mode & S_IFDIR ? 'd' : '-';

  buf[1] = mode & S_IRUSR ? 'r' : '-';
  buf[2] = mode & S_IWUSR ? 'w' : '-';
  buf[3] = mode & S_IXUSR ? 'x' : '-';

  buf[4] = mode & S_IRGRP ? 'r' : '-';
  buf[5] = mode & S_IWGRP ? 'w' : '-';
  buf[6] = mode & S_IXGRP ? 'x' : '-';

  buf[7] = mode & S_IROTH ? 'r' : '-';
  buf[8] = mode & S_IWOTH ? 'w' : '-';
  buf[9] = mode & S_IXOTH ? 'x' : '-';
  buf[10] = 0;
  return buf;
}

static int lstree(CommandLineParser& c)
{
  string input;
  c.po.add_options()
    ("input,i", bpo::value<string>(&input), "input file");
  c.parse_options();

  Tree tree;
  try {
    int fd = open(input.c_str(), O_RDONLY);
    if (fd == -1) {
      cerr << "Failed to open file: " << input << endl;
      return 1;
    }

    FileInputStream fis(fd);
    fis.SetCloseOnDelete(true);
    tree.deserialize(&fis);
  } catch (InvalidTreeException& ex) {
    cerr << "Failed to parse tree: " << ex.what() << endl;
    return 1;
  }

  cout << "References:" << endl;
  const vector<Sha>& refs = tree.get_refs();
  for (int i = 0; i < refs.size(); i++) {
    char b16[SHA_BITS/4 + 1];
    refs[i].base16(b16);
    cout.width(8);
    cout << right << i << ": " << b16 << endl;
  }
  cout << endl;
  cout << "Contents:" << endl;
  char b16[SHA_BITS/4 + 1];
  char fileb16[SHA_BITS/4 + 1];
  memset(fileb16, '-', sizeof(fileb16));
  fileb16[SHA_BITS/4] = 0;
  for (int i = 0; i < tree.get_pb().subdirs_size(); i++) {
    const pb::TreeRef& tr = tree.get_pb().subdirs(i);
    cout << mode_str(tr.mode()) << " " << refs[tr.sha_idx()].base16(b16) << " " << tr.name() << endl;
  }
  for (int i = 0; i < tree.get_pb().files_size(); i++) {
    const pb::FileEntry& fe = tree.get_pb().files(i);
    cout << mode_str(fe.mode()) << " " << fileb16 << " " << fe.name() << endl;
    for (int j = 0; j < fe.parts_size(); j++) {
      const pb::Part& pt = fe.parts(j);
      cout << mode_str(0) << " " << refs[pt.sha_idx()].base16(b16) << " [" << pt.offset() << "--" << pt.offset() + pt.size() << "]" << endl;
    }
  }

  return 0;
}

LYEKKA_LL_COMMAND(lstree, "lstree", "", "Lists a tree's contents");

