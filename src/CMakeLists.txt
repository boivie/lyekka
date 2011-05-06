# Include the directory itself as a path to include directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(lyekka_SOURCES 
server.cpp
chunk_db.cpp
chunk_id.cpp
sha1.c
pack.cpp
)

ADD_EXECUTABLE(lyekka ${lyekka_SOURCES})

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES("../external/sparsehash-1.10/src")

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/CMake/")

find_package(Boost REQUIRED)
find_package(Boost COMPONENTS system filesystem program_options thread REQUIRED)
find_package(Event REQUIRED)

MESSAGE("Boost information:") 
MESSAGE("  Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}") 
MESSAGE("  Boost_LIBRARIES: ${Boost_LIBRARIES}") 
MESSAGE("  Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}") 
include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(lyekka 
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${EVENT_LIBRARIES}
)

