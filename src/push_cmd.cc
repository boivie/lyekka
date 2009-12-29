#include <iostream>
#include <fstream>
#include <boost/filesystem/path.hpp>
#include "cmd_handler.h"
#include "sdsqlite/sdsqlite.h"
#include "formatter.h"
#include "indexed_obj.h"
#include "db.h"
#include "foldermap.h"
#include "remotes.h"
#include "chunkfactory.h"
#include <sys/time.h>

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

void print_vector(string name, vector<uint8_t>& vec)
{
  cout << name << ": ";
  for (int i = 0; i < vec.size(); i++)
  {    
    printf("%02X", vec[i]);
  }
  cout << endl;
}

void update_chunk(sd::sqlite& db, int64_t id, Chunk& chunk, int64_t file_id)
{
  vector<uint8_t> sha(SHA_SIZE_BYTES);
  vector<uint8_t> key(SHA_SIZE_BYTES);

  chunk.get_sha(&sha[0]);
  chunk.get_key(&key[0]);
  
  try {
    sd::sql query(db);
    query << "UPDATE OR ABORT chunks SET sha=?, key=? WHERE id=?";
    query << sha << key << id;
    query.step();
  } 
  catch (sd::db_error& err)
  {
    // Constraint failed. That means that we have a chunk with this checksum already. Find it and use it.
    int64_t new_id;
    sd::sql selquery(db);
    selquery << "SELECT id FROM chunks WHERE key = ?";
    selquery << key;
    selquery.step();
    selquery >> new_id;

    sd::sql updquery(db);
    updquery << "UPDATE local_mapping SET chunk_id = ? WHERE file_id = ? AND chunk_id = ?";
    updquery << new_id << file_id << id;
    updquery.step();
  }
}

string get_rate(uint64_t size)
{
  string ret;
  static uint64_t last_size = 0;
  static struct timeval last_tv = {0};
  struct timeval tv;
  gettimeofday(&tv, NULL);

  if (last_size != 0)
  {
    uint64_t size_diff = size - last_size;
    uint64_t time_diff;
    const int divisor = 1000000/1024;
    time_diff = (tv.tv_sec - last_tv.tv_sec) * 1024;
    time_diff += (int)(tv.tv_usec - last_tv.tv_usec) / divisor;
    ret = Formatter::format_rate(size_diff / time_diff);
  }
  else  
  {
    ret = "-- KiB/s";
  } 

  memcpy(&last_tv, &tv, sizeof(tv));
  last_size = size; 
  return ret;
}

void get_count(sd::sqlite& db, int remote_id, size_t& count, uint64_t& sum_size)
{
  sd::sql countq(db);
  countq << "SELECT COUNT(c.id), SUM(c.size) " 
    "FROM chunks c, local_mapping l, files f "
    "WHERE l.file_id=f.id AND c.id=l.chunk_id AND f.type = ? "
    "AND c.id NOT IN (SELECT chunk_id FROM remote_mapping WHERE remote_id = ?) "
    "ORDER BY f.mtime ASC";
  countq << INDEXED_TYPE_FILE << remote_id;
  countq.step();
  countq >> count >> sum_size;
}

int CMD_push::execute(int argc, char* argv[])
{
  FolderMap fm;
  fm.load();
  
  if (argc < 2)
    throw PushCmdUsageException();

  size_t count, i = 0;
  uint64_t cur_size, sum_size;
  string remote(argv[1]);
  try {
    RemoteInfo remote_info = Remotes::get(remote);

    sd::sqlite& db = Db::get();
    get_count(db, remote_info.id, count, sum_size);
    cout << "Copying a maximum of " << count << " objects, " << Formatter::format_size(sum_size) << " to '" << remote_info.name << "'." << endl;
    
    sd::sql query(db);  
    query << "SELECT c.id, c.sha, c.size, l.offset, f.name, f.parent, f.mtime, f.id " 
      "FROM chunks c, local_mapping l, files f "
      "WHERE l.file_id=f.id AND c.id=l.chunk_id AND f.type = ? "
      "AND c.id NOT IN (SELECT chunk_id FROM remote_mapping WHERE remote_id = ?) "
      "ORDER BY f.mtime ASC";
    query << INDEXED_TYPE_FILE << remote_info.id;
    while (query.step())
    {
      int64_t id, offset, file_id, parent_id;
      string sha, name;
      int size;
      time_t mtime;
      query >> id >> sha >> size >> offset >> name >> parent_id >> mtime >> file_id;
      boost::filesystem::path path;
      fm.build_path(parent_id, path);
      path /= name;
      // TODO: Verify that the mtime is correct
      std::ofstream out("/dev/null");
      Chunk chunk = ChunkFactory::create(path, offset, size, out); 
      update_chunk(db, id, chunk, file_id);
      // TODO: Write to file  
      i++;
      cur_size += size;

      cout << "                      \rProcessed " << i << "/" << count << " (" << Formatter::format_size(cur_size) 
           << " | " << get_rate(cur_size) 
           << ")" << flush;
    }
    cout << endl << "Done." << endl;
  } 
  catch (NoSuchRemoteException& e)
  {
    throw PushCmdUsageException("Remote site not found: '" + remote + "'");
  }

  return 0;
}

