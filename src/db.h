#ifndef DB_H_INCLUSION_GUARD
#define DB_H_INCLUSION_GUARD
#include "sdsqlite/sdsqlite.h"

namespace Lyekka {

class Db {
public:
  static sd::sqlite& get(void);
  static void create(void);
private:
  static sd::sqlite m_db;
  static int loaded;
};
}
#endif
