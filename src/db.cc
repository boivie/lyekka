#include <db.h>
#include "sdsqlite/sdsqlite.h"

using namespace Lyekka;


sd::sqlite& Db::get(void)
{
  if (!loaded)
  {
    m_db.open("lyekka.db");
  }
  return m_db;
}

void Db::create(void)
{
  
}

sd::sqlite Db::m_db;
int Db::loaded = 0;
