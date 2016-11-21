cmake_minimum_required(VERSION 3.3)

include_directories(/usr/include/leveldb)
include_directories(libsocket/headers)
add_subdirectory(libsocket)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/lib)

#daemon executable
project(leveldb_daemon)
include_directories(${leveldb_daemon_SOURCE_DIR}/include)

set(DAEMON_SOURCE_FILES
        src/leveldb_daemon.cpp
        src/ipc/Server.cpp
        src/ipc/Command.cpp
        src/storage/Database.cpp
        src/util/Util.cpp)

add_executable(leveldb_daemon ${DAEMON_SOURCE_FILES})
set_property(TARGET leveldb_daemon PROPERTY CXX_STANDARD 14)
set_property(TARGET leveldb_daemon PROPERTY CXX_STANDARD_REQUIRED ON)

add_dependencies(leveldb_daemon socket++)
target_link_libraries(leveldb_daemon socket++)
target_link_libraries(leveldb_daemon leveldb)

#shared library for client
project(leveldb_client)
include_directories(${leveldb_client_SOURCE_DIR}/include)

set(LIB_SOURCE_FILES
        src/ipc/Client.cpp
        src/ipc/Command.cpp
        src/DB.cpp
        include/Serializer.hpp
        src/util/Util.cpp)

add_library(leveldb_client STATIC ${LIB_SOURCE_FILES})
set_property(TARGET leveldb_client PROPERTY CXX_STANDARD 14)
set_property(TARGET leveldb_client PROPERTY CXX_STANDARD_REQUIRED ON)

add_dependencies(leveldb_client socket++)
target_link_libraries(leveldb_client socket++)
