#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>

#include <iostream>
#include <cstring>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/event_struct.h>
#include <event2/util.h>

#include "chunk_id.h"
#include "chunk_db.h"
#include "pack.h"

using namespace std;

ChunkDatabase db;
long long num_requests;

static inline int begins_with(const char* haystack, const char* needle) {
  return strncmp(haystack, needle, strlen(needle)) == 0;
}

static void free_buf(const void *data, size_t datalen, void *extra) {
  free(extra);
}

static void handle_chunk(struct evhttp_request *req, const char *path) {
  struct evbuffer *evb = NULL;
  ChunkT::const_iterator it;
  ChunkId cid;
  char *buf;

  // Syntax: /7a6fcd13fc74e995faa1885ca6be37a5dcb7bc60
  if (strlen(path) != 41) {
    evhttp_send_error(req, 404, 0);
    return;
  }

  cid = ChunkId::from_b16(path + 1);

  ChunkFindResult result = db.find(cid);

  evhttp_add_header(evhttp_request_get_output_headers(req),
		    "Content-Type", "text/plain");
  
  evb = evbuffer_new();
  buf = (char*)malloc(result.size());
  lseek(result.pack().fd(), result.offset(), SEEK_SET);
  read(result.pack().fd(), buf, result.size());
  const char* payload = buf + ntohl(*(uint32_t*)(buf + 40));
  uint32_t payload_size = ntohl(*(uint32_t*)(buf + 36));
  evbuffer_add_reference(evb, payload, payload_size, free_buf, buf);
  
  evhttp_send_reply(req, 200, "OK", evb);
  if (evb)
    evbuffer_free(evb);
}

static void handle_request(struct evhttp_request *req, void *arg)
{
  const char *uri = evhttp_request_get_uri(req);
  struct evhttp_uri *decoded;
  const char *path;

  num_requests++;

  if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
    return;
  }

  decoded = evhttp_uri_parse(uri);
  if (!decoded) {
    evhttp_send_error(req, HTTP_BADREQUEST, 0);
    return;
  }

  path = evhttp_uri_get_path(decoded);
  if (!path) path = "/";

  if (begins_with(path, "/chunks/")) {
    handle_chunk(req, path + 7);
  } else {
    evhttp_send_error(req, 400, "Not Found");
  }
  
  evhttp_uri_free(decoded);
}


int main(int argc, char **argv)
{
  struct event_base *base;
  struct evhttp *http;
  struct evhttp_bound_socket *handle;
  unsigned short port = 8080;

  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    return (1);

  string pack_path(argv[1]);
  db.load(pack_path);
  cout << "Loaded " << db.pack_count() << " packs with "
       << db.chunk_count() << " chunks." << endl;

  base = event_base_new();
  if (!base) {
    cerr << "Couldn't create event_base" << endl;
    return 1;
  }

  http = evhttp_new(base);
  if (!http) {
    cerr << "Couldn't create evhttp" << endl;
    return 1;
  }

  evhttp_set_gencb(http, handle_request, NULL);

  handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
  if (!handle) {
    cerr << "Couldn't bind to port " << port << endl;
    return 1;
  }

  event_base_dispatch(base);
  return 0;
}
