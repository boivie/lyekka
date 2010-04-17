# Include the directory itself as a path to include directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(lyekka_SOURCES 
blob.cc			index_cmd.cc		purge_cmd.cc
blob_cmd.cc		indexer.cc		push_cmd.cc
cmd_handler.cc		ls_files_cmd.cc		remote_cmd.cc
create_cmd.cc		lstree_cmd.cc		remotes.cc
db.cc			lyekka.pb.cc		sha.cc
foldermap.cc		lyekka_impl.pb.cc	status_cmd.cc
formatter.cc		main.cc			tree.cc
gc_cmd.cc		mktree_cmd.cc		wrappers.cc
gen_objects_cmd.cc	path_cmd.cc
hash_stream.cc		paths.cc unpack_cmd.cc file_system_iterator.cc
file.cc archive.cc archive_cmd.cc
mmap_stream.cc object_generator.cc hash_cmd.cc enc_cmd.cc aes_stream.cc
zlib_cmd.cc)

set(sdsqlite_SOURCES sdsqlite/sdsqlite.cpp)
set(sha2_SOURCES sha2/sha2.c)

ADD_EXECUTABLE(lyekka ${lyekka_SOURCES} ${sdsqlite_SOURCES} ${sha2_SOURCES})

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/CMake/")

find_package(Boost REQUIRED)
find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
find_package(Protobuf)
find_package(EXPAT)
find_package(Sqlite3)
find_package(ZLIB)
find_package(OpenSSL)

MESSAGE("Boost information:") 
MESSAGE("  Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}") 
MESSAGE("  Boost_LIBRARIES: ${Boost_LIBRARIES}") 
MESSAGE("  Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}") 
include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(lyekka 
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${PROTOBUF_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${OPENSSL_LIBRARIES}
)

