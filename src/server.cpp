#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <map>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>

class Pack {
public:
  Pack(uint64_t num, int fd) : m_fd(fd), m_num(num), m_ref_cnt(0) {};
  int get_fd() const { return m_fd; };
private:
  int m_ref_cnt;
  int m_fd;
  const uint64_t m_num;
};

typedef std::map<long long, Pack> PackT;

class ChunkId {
public:
  uint8_t cid[20];
  friend bool operator < (const ChunkId& n2, const ChunkId& n1) { return memcmp(n1.cid, n2.cid, 20) > 0; }
};

class ChunkLocation {
public:
  ChunkLocation(uint64_t pack, uint32_t offset, uint32_t size)
    : m_location(pack << (32 - 5) | offset >> 5), m_size(size) {};
  uint64_t get_pack() const { return m_location >> (32 - 6); };
  uint32_t get_offset() const { return (uint32_t)m_location << 5; };
  uint32_t get_size() const { return m_size; }
private:
  const uint64_t m_location;
  const uint32_t m_size;
};

typedef std::map<ChunkId, ChunkLocation> ChunkT;
ChunkT chunks;
PackT packs;

int base16_decode(int c) {
  switch (c) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    return c - '0';
  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    return c - 'a' + 10;
  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    return c - 'A' + 10;
  default:
    return 0;
  }
}

static void handle_request(struct evhttp_request *req, void *arg)
{
	struct evbuffer *evb = NULL;
	const char *uri = evhttp_request_get_uri(req);
	struct evhttp_uri *decoded = NULL;
	const char *path;

	if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
		return;
	}

	/* Decode the URI */
	decoded = evhttp_uri_parse(uri);
	if (!decoded) {
		printf("It's not a good URI. Sending BADREQUEST\n");
		evhttp_send_error(req, HTTP_BADREQUEST, 0);
		return;
	}

	/* Let's see what path the user asked for. */
	path = evhttp_uri_get_path(decoded);
	if (!path) path = "/";

	ChunkT::const_iterator it;
       
	// Syntax: /7a6fcd13fc74e995faa1885ca6be37a5dcb7bc60
	if (strlen(path) != 41) {
	  evhttp_send_error(req, 404, 0);
	  return;
	}

	ChunkId cid;
	char *buf;
	for (int i = 0; i < 20; i++) {
	  int c1 = base16_decode(*(path + 2*i + 1));
	  int c2 = base16_decode(*(path + 2*i + 2));
	  cid.cid[i] = c1 << 4 | c2;
	}

	it = chunks.find(cid);
	if (it == chunks.end()) {
	  return;
	}
	
	//printf("Chunk is at pack %lld, offset %u, size %u\n",
	//       pack, offset, it->second.size);

	evhttp_add_header(evhttp_request_get_output_headers(req),
			  "Content-Type", "text/plain");

	evb = evbuffer_new();
	buf = (char*)malloc(it->second.get_size());
	PackT::const_iterator pit = packs.find(it->second.get_pack());
	lseek(pit->second.get_fd(), it->second.get_offset(), SEEK_SET);
	read(pit->second.get_fd(), buf, it->second.get_size());
	const char* payload = buf + ntohl(*(uint32_t*)(buf + 40));
	uint32_t payload_size = ntohl(*(uint32_t*)(buf + 36));

	evbuffer_add(evb, payload, payload_size);
	free(buf);

	evhttp_send_reply(req, 200, "OK", evb);
	goto done;
err:
	evhttp_send_error(req, 404, "Document was not found");
done:
	if (decoded)
		evhttp_uri_free(decoded);
	if (evb)
		evbuffer_free(evb);
}

static void load_indexes(const char *path) {
  for (PackT::const_iterator i = packs.begin(); i != packs.end(); ++i) {
    char sb[32];
    char fname[PATH_MAX];
    sprintf(fname, "%spack-%012lld.idx", path, i->first);
    
    FILE* fp = fopen(fname, "rb");
    if (!fp) {
      fprintf(stderr, "Failed to open file\n");
      continue;
    }
    if (fread(sb, 1, 32, fp) != 32) {
      fprintf(stderr, "Failed to read index superblock\n");
      fclose(fp);
      continue;
    }
    char entry[32];
    while (fread(entry, 1, 32, fp) == 32) {
      uint32_t offset = ntohl(*(uint32_t*)(entry + 20));
      uint32_t size = ntohl(*(uint32_t*)(entry + 24));
      uint32_t flags = ntohl(*(uint32_t*)(entry + 28));
      ChunkLocation loc(i->first, offset, size);
      ChunkId cid;
      memcpy(cid.cid, entry, 20);
      chunks.insert(std::make_pair(cid, loc));
    }
  }
}

static void find_indexes(const char *path) {
  DIR *dirp = opendir(path);
  struct dirent *dent;
  if (!dirp) {
    fprintf(stderr, "Couldn't open pack directory\n");
    exit(1);
  }

  char fname[PATH_MAX];
  while ((dent = readdir(dirp)) != NULL) {
    // format: pack-123456789012.idx = 21 characters
    if (strlen(dent->d_name) != 21) continue;
    if (strncmp(dent->d_name, "pack-", 5) ||
	strncmp(dent->d_name + 17, ".idx", 4)) continue;
    uint64_t num = (uint64_t)strtoull(dent->d_name + 5, NULL, 10);
    if (num == 0 && errno == EINVAL) continue;
    sprintf(fname, "%spack-%012lld", path, num);

    Pack p(num, open(fname, O_RDONLY));
    packs.insert(std::make_pair(num, p));
  }
 
  closedir(dirp);
}

void dump_all(void) {
  for (ChunkT::const_iterator it = chunks.begin(); it != chunks.end(); ++it) {
    printf("has: ");
    for (int i = 0; i < 20; i++)
      printf("%02x", it->first.cid[i]);
    printf("\n");
  }
}

int
main(int argc, char **argv)
{
	struct event_base *base;
	struct evhttp *http;
	struct evhttp_bound_socket *handle;

	unsigned short port = 8080;
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return (1);

	find_indexes(argv[1]);
	load_indexes(argv[1]);
	printf("Loaded %lu packs with %lu chunks\n", packs.size(), chunks.size());
	//dump_all();

	base = event_base_new();
	if (!base) {
		fprintf(stderr, "Couldn't create an event_base: exiting\n");
		return 1;
	}

	/* Create a new evhttp object to handle requests. */
	http = evhttp_new(base);
	if (!http) {
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");
		return 1;
	}

	/* We want to accept arbitrary requests, so we need to set a "generic"
	 * cb.  We can also add callbacks for specific paths. */
	evhttp_set_gencb(http, handle_request, NULL);

	/* Now we tell the evhttp what port to listen on */
	handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
	if (!handle) {
		fprintf(stderr, "couldn't bind to port %d. Exiting.\n",
		    (int)port);
		return 1;
	}

	event_base_dispatch(base);

	return 0;
}
