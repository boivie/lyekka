#include "lyekka.h"
#include "blob.h"
#include <zlib.h>
#include <sys/mman.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/coded_stream.h>
#include "hash_stream.h"
#include <google/protobuf/io/gzip_stream.h>

using namespace Lyekka;
using namespace std;
using namespace google::protobuf::io;

#define SMALL_FILE_SIZE 32*1024

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

  for ( ;; ) {
    void* dest_p;
    const void* src_p;
    int src_size, dest_size;
    if (!is_p->Next(&src_p, &src_size)) break;
    if (!os_p->Next(&dest_p, &dest_size)) break;
    int bytes = (src_size < dest_size) ? src_size : dest_size;
    memcpy(dest_p, src_p, bytes);
    is_p->BackUp(src_size - bytes);
    os_p->BackUp(dest_size - bytes);
  }

  gzos.Close();
  ret.m_hash = hos.get_digest();
  return ret;
}

void Blob::unpack(google::protobuf::io::ZeroCopyInputStream* is_p, 
		  google::protobuf::io::ZeroCopyOutputStream* os_p)
{
  z_stream stream;
  int ret;
  const void* mem_p;
  int size;

  if (!is_p->Next(&mem_p, &size) ||
      size < 4 ||
      memcmp(mem_p, "blob", 4) != 0)
    throw RuntimeError();

  is_p->BackUp(size - 4);
  memset(&stream, 0, sizeof(stream));
  inflateInit(&stream);
  do {
    int src_size;
    const void* src_p;
    if (!is_p->Next(&src_p, &src_size));
      break;
    stream.next_in = (Bytef*)src_p;
    stream.avail_in = src_size;
    os_p->Next((void**)&stream.next_out, (int*)&stream.avail_out);
    ret = inflate(&stream, Z_NO_FLUSH);
    os_p->BackUp(stream.avail_out);
    os_p->BackUp(stream.avail_in);
  } while (ret == Z_OK);

  deflateEnd(&stream);
}
