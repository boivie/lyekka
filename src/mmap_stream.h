#ifndef MMAP_STREAM_H_INCLUSION_GUARD
#define MMAP_STREAM_H_INCLUSION_GUARD

#include <google/protobuf/io/zero_copy_stream.h>

namespace Lyekka {

  class MmapInputStream : public google::protobuf::io::ZeroCopyInputStream {
  public:
    MmapInputStream(int fd, off_t offset, size_t size);
    virtual ~MmapInputStream();
    
    // implements ZeroCopyInputStream ---------------------------------
    virtual bool Next(const void** data, int* size);

    virtual void BackUp(int count);
    virtual bool Skip(int count);

    virtual google::protobuf::int64 ByteCount() const;

  private:
    off_t m_page_offset;
    size_t m_size;
    void* m_mmap_p;
    char* m_cur_p;
    size_t m_size_left;
  };

}
#endif
