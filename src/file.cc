#include <fcntl.h>
#include <iostream>
#include "file.h"
#include "lyekka.h"
#include <errno.h>
#include <unistd.h>

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;

boost::shared_ptr<ObjectInputStream> FileReader::find(const Sha& sha) const
{
  char buf[256/4 + 1];
  sha.base16(buf);
  int fd = open(buf, O_RDONLY);
  if (fd == -1) {
    cerr << "Failed to open file: " << strerror(errno);
    throw RuntimeError();
  }
  boost::shared_ptr<FileObjectInputStream> fois_p(new FileObjectInputStream(fd, sha));
  return fois_p;
}

void FileWriter::commit(const Sha& sha) {
  char buf[256/4 + 1];
  sha.base16(buf);
  m_fos_p->Close();
  m_fos_p.release();
  if (access(buf, R_OK) == 0) {
    // Already existing!
    unlink(m_tmpname);
  } else {
    if (rename(m_tmpname, buf) != 0) {
      throw RuntimeError();
    }
  }
}

google::protobuf::io::ZeroCopyOutputStream& FileWriter::get_writer()
{
  strcpy(m_tmpname, "lytmp-XXXXXX");
  int fd = mkstemp(m_tmpname);
  if (fd == -1) 
    throw RuntimeError();

  m_fos_p.reset(new FileOutputStream(fd));
  m_fos_p->SetCloseOnDelete(true);
  return *m_fos_p;
}

FileObjectInputStream::~FileObjectInputStream()
{
}

