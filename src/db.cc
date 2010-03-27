#include <unistd.h>
#include <db.h>
#include "sdsqlite/sdsqlite.h"
#include "lyekka.h"

using namespace Lyekka;

sd::sqlite& Db::get(void)
{
  if (!loaded)
  {
    if (access("lyekka.db", F_OK) != 0)
      throw NoDbException();
    m_db.open("lyekka.db");
  }
  return m_db;
}

void Db::create(void)
{
  
}

sd::sqlite Db::m_db;
int Db::loaded = 0;
