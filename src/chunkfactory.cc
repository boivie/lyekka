#include <fstream>
#include "chunkfactory.h"
#include "botan/botan.h"
#include "botan/cbc.h"

using namespace Lyekka;
using namespace std;

Botan::byte buffer[8192];

void feed_pipe(Botan::Pipe& pipe, std::istream& in, uint64_t offset, size_t size)
{
  pipe.start_msg(); 
  in.seekg(offset, ios_base::beg);
  size_t remaining = size;
  while (remaining) {
    in.read((char*)buffer, sizeof(buffer));
    size_t read = in.gcount();
    pipe.write(buffer, read);
    remaining -= read;
    if (read == 0) break;
  }
  pipe.end_msg();  
}

Chunk ChunkFactory::create(boost::filesystem::path& path, uint64_t offset, size_t size, std::ostream& out)
{
  Botan::Pipe plainPipe(
    new Botan::Hash_Filter("SHA-256"));

  std::ifstream in(path.string().c_str(), ios_base::in | ios_base::binary);

  feed_pipe(plainPipe, in, offset, size);

  Botan::byte key_buf[SHA_SIZE_BYTES];
  plainPipe.read(key_buf, SHA_SIZE_BYTES);
  
  Botan::SymmetricKey key(&key_buf[0], SHA_SIZE_BYTES/2); 
  Botan::InitializationVector iv(&key_buf[SHA_SIZE_BYTES/2], SHA_SIZE_BYTES/2);

  Botan::Pipe encPipe(
    Botan::get_cipher("AES-128/CBC/PKCS7", key, iv, Botan::ENCRYPTION),
    new Botan::Fork(
      new Botan::DataSink_Stream(out),
      new Botan::Hash_Filter("SHA-256")));

  feed_pipe(encPipe, in, offset, size);
  in.close();

  Botan::byte sha_buf[SHA_SIZE_BYTES];
  encPipe.read(sha_buf, SHA_SIZE_BYTES, 1);

  Chunk chunk((uint8_t*)sha_buf, (uint8_t*)key_buf, offset, size);
  //  Std::cout << path.string() << "[" << offset << "--" << size << "] = " << "" << endl; 
  return chunk;
}
