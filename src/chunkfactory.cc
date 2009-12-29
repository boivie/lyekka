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

ChunkHash ChunkFactory::generate_hash(std::istream& in, uint64_t offset, size_t size)
{
  Botan::Pipe plainPipe(new Botan::Hash_Filter("SHA-256"));

  feed_pipe(plainPipe, in, offset, size);

  uint8_t hash[SHA_SIZE_BYTES];
  plainPipe.read(hash, SHA_SIZE_BYTES);
  return ChunkHash(hash);
}

Chunk ChunkFactory::generate_chunk(std::istream& in, uint64_t offset, size_t size, const ChunkHash& key, std::ostream& out)
{
  const vector<uint8_t>& key_vec = key.get_hash();
  Botan::SymmetricKey enc_key(&key_vec[0], SHA_SIZE_BYTES/2); 
  Botan::InitializationVector enc_iv(&key_vec[SHA_SIZE_BYTES/2], SHA_SIZE_BYTES/2);

  Botan::Pipe encPipe(
    Botan::get_cipher("AES-128/CBC/PKCS7", enc_key, enc_iv, Botan::ENCRYPTION),
    new Botan::Fork(
      new Botan::DataSink_Stream(out),
      new Botan::Hash_Filter("SHA-256")));

  feed_pipe(encPipe, in, offset, size);

  uint8_t hash[SHA_SIZE_BYTES];
  encPipe.read(hash, SHA_SIZE_BYTES, 1);
  ChunkHash sha(hash);

  Chunk chunk(sha, key, offset, size);
  return chunk;
}
