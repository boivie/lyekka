# what flags you want to pass to the C compiler & linker
AM_CFLAGS = --pedantic -Wall -O2
AM_LDFLAGS = 
INCLUDES = -I../extern/sdsqlite/

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = lyekka
lyekka_SOURCES = \
main.cc index_cmd.cc main.h index_cmd.h cmd_handler.h create_cmd.h create_cmd.cc cmd_handler.cc path_cmd.cc path_cmd.h ../extern/sdsqlite/sdsqlite.cpp



