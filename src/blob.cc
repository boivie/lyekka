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

void Blob::init_from_mem(const void* mem_p, uint32_t size, ZeroCopyOutputStream* os_p)
{
  Sha256OutputStream hos(os_p);
  void* dest_p;
  int dest_size;

  if (!write_to_stream(&hos, "blob", 4))
    throw runtime_error("Failed to write to stream");
  
  GzipOutputStream::Options o;
  o.format = GzipOutputStream::ZLIB;
  GzipOutputStream gzos(&hos, o);

  if (!write_to_stream(&gzos, mem_p, size))
    throw runtime_error("Failed to write to stream");

  gzos.Close();
  m_hash = hos.get_digest();
}

int unpack_from_mem(const void* mem_p, uint32_t size, ZeroCopyOutputStream* os_p)
{
  z_stream stream;
  int ret;

  if (memcmp(mem_p, "blob", 4) != 0)
    return 0;

  memset(&stream, 0, sizeof(stream));
  inflateInit(&stream);
  stream.next_in = (Bytef*)mem_p + 4;
  stream.avail_in = size - 4;
  do {
    os_p->Next((void**)&stream.next_out, (int*)&stream.avail_out);
    size_t consumed_bytes = stream.avail_out;
    ret = inflate(&stream, Z_NO_FLUSH);
    // TODO: Check for errors
    os_p->BackUp(stream.avail_out);
  } while (ret == Z_OK);

  deflateEnd(&stream);
  return 0;
}

Blob Blob::create_from_fd(int in_fd, uint64_t offset, uint32_t size, ZeroCopyOutputStream* os_p)
{
  Blob ret;
  unsigned char sha1[20];

  void *buf = xmmap(NULL, size, PROT_READ, MAP_PRIVATE, in_fd, offset);
  ret.init_from_mem(buf, size, os_p);
  munmap(buf, size);
  return ret;
}

void Blob::unpack_from_fd(int in_fd, uint32_t size, ZeroCopyOutputStream* os_p)
{
  void *buf = xmmap(NULL, size, PROT_READ, MAP_PRIVATE, in_fd, 0);
  unpack_from_mem(buf, size, os_p);
  munmap(buf, size);
}
