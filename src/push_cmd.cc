#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "cmd_handler.h"
#include "sdsqlite/sdsqlite.h"
#include "formatter.h"
#include "indexed_obj.h"
#include "db.h"
#include "foldermap.h"
#include "remotes.h"
#include "chunkfactory.h"
#include <sys/time.h>
#include "cmd_parser.h"

using namespace std;
using namespace Lyekka;
namespace bpo = boost::program_options;
namespace fs = boost::filesystem;

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

bool set_as_duplicate(sd::sqlite& db, int64_t chunk_id, const ChunkHash& key, int64_t file_id, uint64_t offset)
{
  sd::sql selquery(db);
  selquery << "SELECT id FROM chunks WHERE key = ?";
  selquery << key.get_hash();
  if (selquery.step())
  {
    int64_t new_id;
    selquery >> new_id;

    sd::sql updquery(db);
    updquery << "UPDATE local_mapping SET chunk_id = ? WHERE file_id = ? AND chunk_id = ? AND offset = ?";
    updquery << new_id << file_id << chunk_id << offset;
    updquery.step();  
    return true;
  }
  else
  {
    return false;
  }
}

bool update_chunk(sd::sqlite& db, int64_t chunk_id, const Chunk& chunk, int64_t file_id, uint64_t offset)
{
  try {
    sd::sql query(db);
    const vector<uint8_t>& sha = chunk.get_sha().get_hash();
    const vector<uint8_t>& key = chunk.get_key().get_hash();

    query << "UPDATE OR ABORT chunks SET sha=?, key=? WHERE id=?";
    query << sha << key << chunk_id;
    query.step();
    return false;
  } 
  catch (sd::db_error& err)
  {
    // Constraint failed. That means that we have a chunk with this checksum already. Find it and use it.
    return set_as_duplicate(db, chunk_id, chunk.get_key(), file_id, offset);
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
  CmdParser parser(argc, argv);
  parser.parse_command(parser.create_commands()
    ("", "<remote-name> [<destination-path>]"));
  string remote;
  string destination_path;
  bpo::options_description o, po;
  bpo::positional_options_description p;
  po.add_options()
    ("remote_name", bpo::value<string>(&remote), "remote name")
    ("destination_path", bpo::value<string>(&destination_path), "remote name");
  p.add("remote_name", 1);
  p.add("destination_path", 1);
  parser.parse_options(o, po, p);

  if (remote == "")
    throw parser.syntax_exception("missing <remote-name>");

  FolderMap fm;
  fm.load();

  size_t count, i = 0;
  uint64_t cur_size, sum_size;
  try {
    RemoteInfo remote_info = Remotes::get(remote);
    if (destination_path == "")
      destination_path = remote_info.default_destination;

    fs::path destination(destination_path);

    sd::sqlite& db = Db::get();
    get_count(db, remote_info.id, count, sum_size);
    cout << "Copying a maximum of " << count << " objects, " 
         << Formatter::format_size(sum_size) << " to '" << remote_info.name << "'." << endl;
    cout << "Using destination directory: '" << destination << "'" << endl;
    
    sd::sql query(db);  
    query << "SELECT c.id, c.sha, c.size, l.offset, f.name, f.parent, f.mtime, f.id " 
      "FROM chunks c, local_mapping l, files f "
      "WHERE l.file_id=f.id AND c.id=l.chunk_id AND f.type = ? "
      "AND c.id NOT IN (SELECT chunk_id FROM remote_mapping WHERE remote_id = ?) "
      "ORDER BY f.mtime ASC";
    query << INDEXED_TYPE_FILE << remote_info.id;
    int reused = 0;
    while (query.step())
    {
      int64_t chunk_id, offset, file_id, parent_id;
      string sha, name;
      int size;
      time_t mtime;
      query >> chunk_id >> sha >> size >> offset >> name >> parent_id >> mtime >> file_id;
      boost::filesystem::path path;
      fm.build_path(parent_id, path);
      path /= name;
      // TODO: Verify that the mtime is correct
      std::ifstream in(path.string().c_str(), ios_base::in | ios_base::binary);

      ChunkHash key = ChunkFactory::generate_hash(in, offset, size);
      if (set_as_duplicate(db, chunk_id, key, file_id, offset))
      { 
        // Only do this if already at destination
        reused++;
      }

      fs::path tmp_filename("lyekka.tmp");
      fs::path tmp_path = destination_path / tmp_filename;
      std::ofstream out(tmp_path.string().c_str(), ios_base::out | ios_base::binary);
      Chunk chunk = ChunkFactory::generate_chunk(in, offset, size, key, out);
      update_chunk(db, chunk_id, chunk, file_id, offset);
      fs::path new_filename(chunk.get_cid_hex() + ".chunk");
      fs::path out_path = destination_path / new_filename;

      if (!fs::exists(out_path))
      {
        fs::rename(tmp_path, out_path);
      }

      i++;
      cur_size += size;

      // Clear old line and write new one.
      cout << string(80, ' ') << "\r"
           << "Progress: " << i << "/" << count << " (" << Formatter::format_size(cur_size) 
           << " | " << get_rate(cur_size) 
           << ")" << flush;
    } 
    cout << endl << "Done. Total " << i << ", reused " << reused << "." << endl;
    
  } 
  catch (NoSuchRemoteException& e)
  {
    throw parser.syntax_exception("Remote site not found: '" + remote + "'");
  }

  return 0;
}

