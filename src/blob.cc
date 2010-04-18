#include "lyekka.h"
#include "blob.h"
#include <google/protobuf/io/zero_copy_stream.h>
#include "hash_stream.h"
#include <google/protobuf/io/gzip_stream.h>
#include <fcntl.h>

using namespace Lyekka;
using namespace std;
using namespace google::protobuf::io;

auto_ptr<ObjectIdentifier> Blob::create(ZeroCopyInputStream* is_p, 
					ZeroCopyOutputStream* os_p,
					const AesKey* key_p)
{
  Sha256OutputStream hos(os_p);

  if (!write_to_stream(&hos, "blob", 4))
    throw runtime_error("Failed to write to stream");
  
  GzipOutputStream::Options o;
  o.format = GzipOutputStream::ZLIB;

  if (key_p == NULL) {
    GzipOutputStream gzos(&hos, o);
    copy_streams(&gzos, is_p);
    gzos.Close();
    return auto_ptr<ObjectIdentifier>(new ObjectIdentifier(hos.get_digest()));
  } else {
    AesOutputStream aos(&hos, AesOutputStream::CBC, *key_p);
    GzipOutputStream gzos(&aos, o);
    copy_streams(&gzos, is_p);
    gzos.Close();
    aos.Close();

    // TODO: Only support AES128 right now.
    Aes128Key key(key_p->key(), key_p->iv());
    return auto_ptr<ObjectIdentifier>(new Aes128ObjectIdentifier(hos.get_digest(),
								 key));
  }
}

void Blob::unpack(ZeroCopyInputStream* is_p, 
		  ZeroCopyOutputStream* os_p,
		  const AesKey* key_p)
{
  const void* mem_p;
  int size;

  if (!is_p->Next(&mem_p, &size) ||
      size < 4 ||
      memcmp(mem_p, "blob", 4) != 0)
    throw RuntimeError();

  is_p->BackUp(size - 4);

  if (key_p == NULL) {
    GzipInputStream gzis(is_p, GzipInputStream::ZLIB);
    copy_streams(os_p, &gzis);
  } else {
    AesInputStream ais(is_p, AesInputStream::CBC, *key_p);
    GzipInputStream gzis(&ais, GzipInputStream::ZLIB);
    copy_streams(os_p, &gzis);
  }
}

auto_ptr<AesKey> Blob::generate_key(ZeroCopyInputStream* is_p) {
  FileOutputStream fos(open("/dev/null", O_WRONLY));
  fos.SetCloseOnDelete(true);
  Sha256OutputStream hos(&fos);
  copy_streams(&hos, is_p);
  Sha sha = hos.get_digest();
  auto_ptr<AesKey> key_p(new Aes128Key(sha.data(), sha.data() + SHA_BITS/8/2));
  return key_p;
}
