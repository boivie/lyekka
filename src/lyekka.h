#ifndef LYEKKA_H_INCLUSION_GUARD
#define LYEKKA_H_INCLUSION_GUARD

#include <cstring>
#include <stdexcept>
#include <google/protobuf/io/zero_copy_stream.h>

namespace Lyekka {

  class NoDbException : public std::runtime_error {
  public:
    NoDbException() : std::runtime_error("Database required") {};
  };

  class RuntimeError : public std::runtime_error {
  public:
    RuntimeError() : std::runtime_error("Other runtime error") {};
  };
  
  extern void *xmmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
  extern int xopen(const char* fn_p, int oflag, int mode = 0);
  bool write_to_stream(google::protobuf::io::ZeroCopyOutputStream* os_p, const void* src_p, size_t size);
  size_t read_from_stream(void* dest_p, google::protobuf::io::ZeroCopyInputStream* is_p, size_t size);
  void copy_streams(google::protobuf::io::ZeroCopyOutputStream* os_p,
		    google::protobuf::io::ZeroCopyInputStream* is_p);

  unsigned char* base16_decode(unsigned char* dest_p, const char* src_p, int len = -1);
  char* base16_encode(char* dest_p, const unsigned char* src_p, size_t len);
}


#endif
