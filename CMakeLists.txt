cmake_minimum_required(VERSION 3.3)

include_directories(./ ../)
include_directories(/usr/include/leveldb)
include_directories(libsocket/headers)
add_subdirectory(libsocket)

#daemon executable
project(leveldb_daemon)

set(DAEMON_SOURCE_FILES
        leveldb_daemon.cpp
        storage/Database.cpp
        ipc/Server.cpp
        logging/Logger.h)

add_executable(leveldb_daemon ${DAEMON_SOURCE_FILES})
set_property(TARGET leveldb_daemon PROPERTY CXX_STANDARD 14)
set_property(TARGET leveldb_daemon PROPERTY CXX_STANDARD_REQUIRED ON)

add_dependencies(leveldb_daemon socket++)
target_link_libraries(leveldb_daemon socket++)
target_link_libraries(leveldb_daemon leveldb)


#shared library for client
project(leveldb_client)

set(LIB_SOURCE_FILES
        ipc/Client.cpp
        logging/Logger.h
        serializer/Serializer.hpp
        include/DB.cpp)

add_library(leveldb_client STATIC ${LIB_SOURCE_FILES})
set_property(TARGET leveldb_client PROPERTY CXX_STANDARD 14)
set_property(TARGET leveldb_client PROPERTY CXX_STANDARD_REQUIRED ON)

add_dependencies(leveldb_client socket++)
target_link_libraries(leveldb_client socket++)
