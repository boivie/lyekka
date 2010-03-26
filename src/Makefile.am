# what flags you want to pass to the C compiler & linker
AM_CFLAGS = --pedantic -Wall -std=c99
AM_LDFLAGS = 
INCLUDES = 

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = lyekka
lyekka_SOURCES = \
  main.cc \
  cmd_handler.cc \
  indexer.cc \
  db.cc \
  paths.cc  \
  sdsqlite/sdsqlite.cpp \
  remotes.cc  \
  formatter.cc  \
  foldermap.cc  \
  chunkfactory.cc \
  sha2/sha2.c \
  hash_stream.cc \
  blob.cc  \
  wrappers.cc \
  sha.cc  \
  tree.cc \
  lyekka.pb.cc \
  lyekka_impl.pb.cc \
  index_cmd.cc \
  create_cmd.cc \
  path_cmd.cc  \
  status_cmd.cc  \
  remote_cmd.cc \
  push_cmd.cc  \
  gc_cmd.cc \
  purge_cmd.cc \
  ls_files_cmd.cc \
  blob_cmd.cc \
  mktree_cmd.cc \
  lstree_cmd.cc \
  gen_objects_cmd.cc