#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <pthread.h>
#include <execinfo.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/event_struct.h>
#include <event2/http_struct.h>
#include <event2/util.h>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>

#include "chunk_id.h"
#include "chunk_db.h"
#include "pack.h"

using namespace std;
namespace po = boost::program_options;


ChunkDatabase db;
long long num_requests;

class BodyReader {
public:
  BodyReader() : m_buf(new vector<uint8_t>()) {
    m_buf->reserve(4*1024*1024);
  }
  void feed(const void* data, size_t len) {
    size_t offset = m_buf->size();
    m_buf->resize(offset + len);
    memcpy(&(*m_buf.get())[offset], data, len);
  }

  BufferPtr buffer() { return m_buf; }
private:
  BufferPtr m_buf;
};

static inline int begins_with(const char* haystack, const char* needle) {
  return strncmp(haystack, needle, strlen(needle)) == 0;
}

static void handle_put_chunk(struct evhttp_request *req, const char *path) {
  BodyReader* br = (BodyReader*)(req->body_opaque);
  if (br == NULL) {
    evhttp_send_error(req, 500, "No body data");
    return;
  }

  const ChunkId cid = ChunkId::calculate(&(*br->buffer())[0],
					 br->buffer()->size());
  const StoredChunk sc(cid, br->buffer(), 0, br->buffer()->size());
  db.write_chunk(sc);

  cout << "write_chunk " << cid.hex() << " " << br->buffer()->size()
       << " bytes" << endl;
  
  struct evbuffer *evb = evbuffer_new();
  evbuffer_add_printf(evb, "%s", cid.hex().c_str());
  evhttp_send_reply(req, 200, "OK", evb);
  evbuffer_free(evb);
}

class DeferredBuffer {
public:
  DeferredBuffer(BufferPtr buffer) : m_buffer(buffer) {};
private:
  BufferPtr m_buffer;
};

static void free_deferred(const void *data, size_t datalen, void *extra) {
  DeferredBuffer *buf = (DeferredBuffer *)extra;
  delete buf;
}

static void handle_get_chunk(struct evhttp_request *req, const char *path) {
  // Syntax: /7a6fcd13fc74e995faa1885ca6be37a5dcb7bc60
  if (strlen(path) != 41) {
    evhttp_send_error(req, 404, 0);
    return;
  }

  try {
    struct evbuffer *evb = NULL;
    const ChunkId cid = ChunkId::from_hex(path + 1);
    const StoredChunk sc = db.find(cid);

    evhttp_add_header(evhttp_request_get_output_headers(req),
		      "Content-Type", "text/plain");
  
    evb = evbuffer_new();
    const uint8_t* raw_p = &(*sc.raw_buffer())[0];

    // Just used to keep a reference to the buffer.
    DeferredBuffer* deferred = new DeferredBuffer(sc.raw_buffer());

    evbuffer_add_reference(evb, raw_p + sc.payload_offset(),
			   sc.payload_size(), free_deferred, deferred);
  
    cout << "200 get " << path << " " << sc.payload_size() << " bytes" << endl;
    evhttp_send_reply(req, 200, "OK", evb);
    if (evb)
      evbuffer_free(evb);
  } catch (ChunkNotFound& cnf) {
    cout << "404 get " << path << endl;
    evhttp_send_error(req, 404, "Chunk not found");
  }
}

static inline int is_get(struct evhttp_request *req) {
  return evhttp_request_get_command(req) == EVHTTP_REQ_GET;
}

static inline int is_put(struct evhttp_request *req) {
  return evhttp_request_get_command(req) == EVHTTP_REQ_PUT;
}

static void* body_create(struct evhttp_request *req, void *arg) {
  struct evkeyvalq *headers = evhttp_request_get_input_headers(req);
  BodyReader* br = new BodyReader();
  return br;
}

static void body_destroy(struct evhttp_request *req, void *opaque) {
  BodyReader* br = (BodyReader*)opaque;
  delete br;
}

static void body_read(struct evhttp_request *req, void *opaque) {
  char buf[4097];
  BodyReader* br = (BodyReader*)opaque;
  int bytes = evbuffer_remove(req->input_buffer, buf, 4096);
  br->feed(buf, bytes);
}

static void handle_request(struct evhttp_request *req, void *arg)
{
  const char *uri = evhttp_request_get_uri(req);
  struct evhttp_uri *decoded;
  const char *path;

  decoded = evhttp_uri_parse(uri);
  if (!decoded) {
    evhttp_send_error(req, HTTP_BADREQUEST, 0);
    return;
  }

  path = evhttp_uri_get_path(decoded);
  if (!path) path = "/";

  if (is_get(req) && begins_with(path, "/chunk/")) {
    handle_get_chunk(req, path + 6);
  } else if (is_put(req) && begins_with(path, "/chunk/")) {
    handle_put_chunk(req, path + 6);
  } else {
    evhttp_send_error(req, 400, "Not Found");
  }
  
  evhttp_uri_free(decoded);
}

void printStackTrace(ostream &o = cout) {
  void *b[20];
	
  int size = backtrace(b, 20);
  for (int i = 0; i < size; i++)
    o << hex << b[i] << dec << ' ';
  o << endl;
  
  char **strings = backtrace_symbols(b, size);
  for (int i = 0; i < size; i++)
    o << ' ' << strings[i] << '\n';
  o.flush();
  free (strings);
}

void fatal_signal_handler(int x) {
  cout << "Got signal: " << x << " (" << strsignal(x) << ")." << endl;
  ostringstream oss;
  oss << "Backtrace:" << endl;
  printStackTrace( oss );
  cout << oss.str();
}

sigset_t asyncSignals;
void ctrl_c_handler() {
  int x;
  sigwait(&asyncSignals, &x);
  cerr << "Got signal " << x << " (" << strsignal(x)
       << ") - exiting cleanly." << endl;
  db.close_partial();
  exit(0);
}

void myterminate() {
  cout << "terminate() called, printing stack:" << endl;
  printStackTrace();
  abort();
}

void setup_signals() {
  assert(signal(SIGSEGV, fatal_signal_handler) != SIG_ERR);
  assert(signal(SIGFPE, fatal_signal_handler) != SIG_ERR);
  assert(signal(SIGABRT, fatal_signal_handler) != SIG_ERR);
  assert(signal(SIGBUS, fatal_signal_handler) != SIG_ERR);
  assert(signal(SIGQUIT, fatal_signal_handler) != SIG_ERR);
  assert(signal(SIGPIPE, SIG_IGN) != SIG_ERR);

  sigemptyset(&asyncSignals);
  sigaddset(&asyncSignals, SIGHUP);
  sigaddset(&asyncSignals, SIGINT);
  sigaddset(&asyncSignals, SIGTERM);

  assert(pthread_sigmask(SIG_SETMASK, &asyncSignals, 0) == 0);
  boost::thread it(ctrl_c_handler);

  set_terminate(myterminate);
}

int main(int argc, char* argv[])
{
  po::options_description general_options("General options");
  po::variables_map params;

  general_options.add_options()
    ("repair", "repair unclean shutdown")
    ("help", "show help text");

  int style = (((po::command_line_style::unix_style ^
		 po::command_line_style::allow_guessing) |
		po::command_line_style::allow_long_disguise) ^
	       po::command_line_style::allow_sticky);
  
  po::store(po::command_line_parser(argc, argv)
	    .options(general_options)
	    .style(style)
	    .run(), params);
  
  if (params.count("help")) {
    cout << general_options << endl;
    return 0;
  }

  boost::filesystem::path pack_path(argv[1]);
  db.set_path(pack_path);

  if (params.count("repair"))
    db.repair();

  if (!db.load())
    exit(1);
  if (!db.create_partial())
    exit(2);

  struct event_base *base;
  struct evhttp *http;
  struct evhttp_bound_socket *handle;
  unsigned short port = 8080;

  setup_signals();

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

  evhttp_set_body_reader(http, body_create, body_read, body_destroy, NULL);
  evhttp_set_gencb(http, handle_request, NULL);

  handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
  if (!handle) {
    cerr << "Couldn't bind to port " << port << endl;
    return 1;
  }
  cout << "Listening for incoming connections." << endl;
  event_base_dispatch(base);
  return 0;
}

void die() {
  printStackTrace();
  assert(0);
}
