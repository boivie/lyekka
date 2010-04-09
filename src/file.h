#ifndef FILE_H_INCLUSION_GUARD
#define FILE_H_INCLUSION_GUARD

#include "sha.h"
#include "object.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace Lyekka {

  class FileObjectInputStream : public ObjectInputStream {
  public:
    virtual ~FileObjectInputStream();
    // implements ZeroCopyInputStream ---------------------------------
    bool Next(const void** data, int* size) { return m_is.Next(data, size); }
    void BackUp(int count) { m_is.BackUp(count); }
    bool Skip(int count) { return m_is.Skip(count); }
    google::protobuf::int64 ByteCount() const { return m_is.ByteCount(); }
  protected:
    friend class FileReader;
    FileObjectInputStream(int fd, const Sha& sha) : 
      m_is(fd), ObjectInputStream(sha) { m_is.SetCloseOnDelete(true); }
    google::protobuf::io::FileInputStream m_is;
  };

  class FileReader : public ObjectReader {
  public:
    virtual boost::shared_ptr<ObjectInputStream> find(const Sha& sha) const;
  };

  class FileWriter : public ObjectWriter {
  public:
    FileWriter() : m_fos_p(NULL) {}
    void commit(Sha sha);
    google::protobuf::io::ZeroCopyOutputStream& get_writer();
  private:
    std::auto_ptr<google::protobuf::io::FileOutputStream> m_fos_p;
    char m_tmpname[20];
  };

}

#endif
