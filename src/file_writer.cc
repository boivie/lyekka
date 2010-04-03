#include <iostream>
#include "file_writer.h"
#include "lyekka.h"
#include <unistd.h>

using namespace std;
using namespace Lyekka;
using namespace google::protobuf::io;

void FileWriter::commit(Sha sha) {
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
