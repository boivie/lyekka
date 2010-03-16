#include "lyekka.h"
#include "blob.h"
#include <zlib.h>
#include <sys/mman.h>
#include <openssl/sha.h>
using namespace Lyekka;
using namespace std;

#define SMALL_FILE_SIZE 32*1024

void init_from_mem(Blob& blob, const void* mem_p, uint32_t size, int out_fd)
{
  struct {
    char id[4];
  } header;
  unsigned char compressed[4096];
  SHA256_CTX ctx;
	z_stream stream;
  int ret;

  header.id[0] = 'b'; header.id[1] = 'l';
  header.id[2] = 'o'; header.id[3] = 'b';
  // Hash and write the header
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, &header, sizeof(header));
  write(out_fd, &header, sizeof(header));

	deflateInit(&stream, 6);
	stream.next_in = (Bytef*)mem_p;
	stream.avail_in = size;
  stream.next_out = compressed;
  stream.avail_out = sizeof(compressed);
	do {
		ret = deflate(&stream, Z_FINISH);
	  SHA256_Update(&ctx, compressed, stream.next_out - compressed);
		if (write(out_fd, compressed, stream.next_out - compressed) < 0)
			exit(4);
		stream.next_out = compressed;
		stream.avail_out = sizeof(compressed);
	} while (ret == Z_OK);

	ret = deflateEnd(&stream);
  SHA256_Final(blob.hash, &ctx);
}


int unpack_from_mem(const void* mem_p, uint32_t size, int out_fd)
{
  static const char header[4] = {'b', 'l', 'o', 'b'};
  unsigned char decompressed[16384];
	z_stream stream;
  int ret;

  if (memcmp(mem_p, header, sizeof(header)) != 0)
    return 0;

  // Hash and write the header
	inflateInit(&stream);
	stream.next_in = (Bytef*)mem_p + 4;
	stream.avail_in = size - 4;
	do {
    stream.next_out = decompressed;
    stream.avail_out = sizeof(decompressed);
		ret = inflate(&stream, Z_NO_FLUSH);
		if (write(out_fd, decompressed, stream.next_out - decompressed) < 0)
			exit(4);
	} while (ret == Z_OK);

	deflateEnd(&stream);
  return 0;
}


Blob Blob::create_from_fd(int in_fd, uint64_t offset, uint32_t size, int out_fd)
{
  Blob ret;
	unsigned char sha1[20];

	void *buf = xmmap(NULL, size, PROT_READ, MAP_PRIVATE, in_fd, offset);
	init_from_mem(ret, buf, size, out_fd);
	munmap(buf, size);
	return ret;
}

void Blob::unpack_from_fd(int in_fd, uint32_t size, int out_fd)
{
	void *buf = xmmap(NULL, size, PROT_READ, MAP_PRIVATE, in_fd, 0);
	unpack_from_mem(buf, size, out_fd);
	munmap(buf, size);
}
