# what flags you want to pass to the C compiler & linker
AM_CFLAGS = --pedantic -Wall -std=c99
AM_LDFLAGS = 
INCLUDES = 

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = lyekka
lyekka_SOURCES = \
main.cc index_cmd.cc main.h cmd_handler.h create_cmd.cc cmd_handler.cc path_cmd.cc sdsqlite/sdsqlite.cpp sdsqlite/sdsqlite.h indexer.cc indexer.h indexed_obj.h db.h db.cc chunk.h paths.h paths.cc status_cmd.cc remote_cmd.cc remotes.h remotes.cc formatter.h formatter.cc push_cmd.cc foldermap.h foldermap.cc chunkfactory.h chunkfactory.cc gc_cmd.cc purge_cmd.cc generate_index_cmd.cc ls_files_cmd.cc sha2/sha2.h sha2/sha2.c hash_stream.h hash_stream.cc lyekka.pb.h lyekka.pb.cc lyekka_impl.pb.h lyekka_impl.pb.cc blob_cmd.cc blob.h blob.cc wrappers.cc lyekka.h mktree_cmd.cc tree.h sha.h sha.cc tree.cc lstree_cmd.cc