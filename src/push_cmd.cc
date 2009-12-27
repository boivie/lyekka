#include <iostream>
#include <boost/filesystem/path.hpp>
#include "cmd_handler.h"
#include "sdsqlite/sdsqlite.h"

#include "indexed_obj.h"
#include "db.h"
#include "foldermap.h"
#include "remotes.h"
#include "chunkfactory.h"

using namespace std;
using namespace Lyekka;

class PushCmdUsageException : public CmdUsageException
{ 
public:
  PushCmdUsageException(std::string e = "") : CmdUsageException(e) {}
  ~PushCmdUsageException() throw () {};
  void print_usage(CmdUsageStream& os) 
  {
    os << "push <remote>" 
       << "push <remote> <destination-path>";
  }
};

DECLARE_COMMAND(push, "push", "Copies the latest data.");

void update_chunk(sd::sqlite& db, int64_t id, Chunk& chunk)
{
  sd::sql query(db);
  query << "UPDATE chunks SET sha=?, key=? WHERE id=?";
  query << chunk.get_cid() << chunk.get_key() << id;
  query.step();
}

int CMD_push::execute(int argc, char* argv[])
{
  FolderMap fm;
  fm.load();
  
  if (argc < 2)
    throw PushCmdUsageException();

  string remote(argv[1]);
  try {
    RemoteInfo remote_info = Remotes::get(remote);
    
    sd::sqlite& db = Db::get();
    db << "BEGIN EXCLUSIVE";
    sd::sql query(db);  
    query << "SELECT c.id, c.sha, c.size, l.offset, f.name, f.parent, f.mtime " 
      "FROM chunks c, local_mapping l, files f "
      "WHERE l.file_id=f.id AND c.id=l.chunk_id AND f.type = ? "
      "AND c.id NOT IN (SELECT chunk_id FROM remote_mapping WHERE remote_id = ?) "
      "ORDER BY f.mtime ASC";
    query << INDEXED_TYPE_FILE << remote_info.id;
    while (query.step())
    {
      int64_t id;
      string sha;
      int size;
      int64_t offset;
      string name;
      int parent;
      time_t mtime;
      query >> id >> sha >> size >> offset >> name >> parent >> mtime;
      boost::filesystem::path path;
      fm.build_path(parent, path);
      path /= name;
      // TODO: Verify that the mtime is correct
      ChunkBuffer buffer;
      Chunk chunk = ChunkFactory::create(path, offset, size, buffer); 
      update_chunk(db, id, chunk);
      // TODO: Write to file  
      
    }
    db << "COMMIT";
  } 
  catch (NoSuchRemoteException& e)
  {
    throw PushCmdUsageException("Remote site not found: '" + remote + "'");
  }

  return 0;
}

