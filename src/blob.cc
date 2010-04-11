#include "lyekka.h"
#include "blob.h"
#include <google/protobuf/io/zero_copy_stream.h>
#include "hash_stream.h"
#include <google/protobuf/io/gzip_stream.h>

using namespace Lyekka;
using namespace std;
using namespace google::protobuf::io;

Blob Blob::create(google::protobuf::io::ZeroCopyInputStream* is_p, 
		  google::protobuf::io::ZeroCopyOutputStream* os_p)
{
  Blob ret;
  Sha256OutputStream hos(os_p);

  if (!write_to_stream(&hos, "blob", 4))
    throw runtime_error("Failed to write to stream");
  
  GzipOutputStream::Options o;
  o.format = GzipOutputStream::ZLIB;
  GzipOutputStream gzos(&hos, o);

  copy_streams(&gzos, is_p);

  gzos.Close();
  ret.m_hash = hos.get_digest();
  return ret;
}

void Blob::unpack(google::protobuf::io::ZeroCopyInputStream* is_p, 
		  google::protobuf::io::ZeroCopyOutputStream* os_p)
{
  const void* mem_p;
  int size;

  if (!is_p->Next(&mem_p, &size) ||
      size < 4 ||
      memcmp(mem_p, "blob", 4) != 0)
    throw RuntimeError();

  is_p->BackUp(size - 4);

  GzipInputStream gzis(is_p, GzipInputStream::ZLIB);
  copy_streams(os_p, &gzis);
}
