#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

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
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

#ifdef _EVENT_HAVE_NETINET_IN_H
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#endif


class Pack {
public:
  int ref_cnt;
  int fd;
  long long num;
};

typedef std::map<long long, Pack> PackT;

class ChunkId {
public:
  uint8_t cid[20];
  friend bool operator < (const ChunkId& n2, const ChunkId& n1) { return memcmp(n1.cid, n2.cid, 20) > 0; }
};

class ChunkLocation {
public:
  uint64_t location;
  uint32_t size;
};

typedef std::map<ChunkId, ChunkLocation> ChunkT;
ChunkT chunks;
PackT packs;


char uri_root[512];

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

/* This callback gets invoked when we get any http request that doesn't match
 * any other callback.  Like any evhttp server callback, it has a simple job:
 * it must eventually call evhttp_send_error() or evhttp_send_reply().
 */
static void
send_document_cb(struct evhttp_request *req, void *arg)
{
	struct evbuffer *evb = NULL;
	const char *docroot = (char*)arg;
	const char *uri = evhttp_request_get_uri(req);
	struct evhttp_uri *decoded = NULL;
	const char *path;
	char *decoded_path;
	char *whole_path = NULL;
	size_t len;
	int fd = -1;
	struct stat st;

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

	/* We need to decode it, to see what path the user really wanted. */
	decoded_path = evhttp_uridecode(path, 0, NULL);

	ChunkT::iterator it;
       
	// Syntax: /7a6fcd13fc74e995faa1885ca6be37a5dcb7bc60
	if (strlen(decoded_path) != 41) return;

	ChunkId cid;
	uint64_t pack;
	uint32_t offset;
	char *buf;
	for (int i = 0; i < 20; i++) {
	  int c1 = base16_decode(*(decoded_path + 2*i + 1));
	  int c2 = base16_decode(*(decoded_path + 2*i + 2));
	  cid.cid[i] = c1 << 4 | c2;
	}

	it = chunks.find(cid);
	if (it == chunks.end()) {
	  return;
	}
	
	pack = it->second.location >> (32 - 5);
	offset = (uint32_t)it->second.location << 5;
	
	//printf("Chunk is at pack %lld, offset %u, size %u\n",
	//       pack, offset, it->second.size);

	evhttp_add_header(evhttp_request_get_output_headers(req),
			  "Content-Type", "text/plain");

	evb = evbuffer_new();
	buf = (char*)malloc(it->second.size);
	PackT::iterator pit = packs.find(pack);
	lseek(pit->second.fd, offset, SEEK_SET);
	read(pit->second.fd, buf, it->second.size);
	const char* payload = buf + ntohl(*(uint32_t*)(buf + 40));
	uint32_t payload_size = ntohl(*(uint32_t*)(buf + 36));

	evbuffer_add(evb, payload, payload_size);
	free(buf);

	evhttp_send_reply(req, 200, "OK", evb);
	goto done;
err:
	evhttp_send_error(req, 404, "Document was not found");
	if (fd>=0)
		close(fd);
done:
	if (decoded)
		evhttp_uri_free(decoded);
	if (decoded_path)
		free(decoded_path);
	if (whole_path)
		free(whole_path);
	if (evb)
		evbuffer_free(evb);
}

void load_indexes(const char *path) {
  for (PackT::iterator i = packs.begin(); i != packs.end(); ++i) {
    char sb[32];
    char fname[PATH_MAX];
    sprintf(fname, "%spack-%012lld.idx", path, i->first);
    //printf("Handling %lld (%s)\n", i->first, fname);
    
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
      ChunkLocation loc;
      loc.location = i->first << (32 - 5) | offset >> 5;
      loc.size = size;
      ChunkId cid;
      memcpy(cid.cid, entry, 20);
      chunks.insert(std::make_pair(cid, loc));
    }
  }
}

void read_indexes(const char *path) {
  DIR *dirp = opendir(path);
  struct dirent *dent;
  if (!dirp) {
    fprintf(stderr, "Couldn't open pack directory\n");
    exit(1);
  }

  char fname[PATH_MAX];
  while ((dent = readdir(dirp)) != NULL) {
    // format: pack-123456789012.idx = 21
    if (strlen(dent->d_name) != 21) continue;
    if (strncmp(dent->d_name, "pack-", 5) ||
	strncmp(dent->d_name + 17, ".idx", 4)) continue;
    long long num = strtoull(dent->d_name + 5, NULL, 10);
    if (num == 0 && errno == EINVAL) continue;
    Pack p;
    sprintf(fname, "%spack-%012lld", path, num);
    p.fd = open(fname, O_RDONLY);
    if (p.fd < 0) {
      fprintf(stderr, "Failed to open packfile %s\n", fname);
      continue;
    }
    p.num = num;
    packs.insert(std::make_pair(num, p));
  }
 
  closedir(dirp);
}

void dump_all(void) {
  for (ChunkT::iterator it = chunks.begin(); it != chunks.end(); ++it) {
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

	read_indexes(argv[1]);
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
	evhttp_set_gencb(http, send_document_cb, argv[1]);

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
