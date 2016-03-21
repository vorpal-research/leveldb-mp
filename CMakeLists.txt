cmake_minimum_required(VERSION 3.3)

include_directories(./ ../)
include_directories(/usr/include/leveldb)
include_directories(libsocket/headers)

#daemon executable
project(leveldb_daemon)

set(SOURCE_FILES
        leveldb_daemon.cpp
        storage/Database.cpp
        ipc/Server.cpp
        ipc/Client.cpp
        logging/Logger.h
        serializer/Serializer.hpp
        include/DB.hpp)

add_executable(leveldb_daemon ${SOURCE_FILES})
set_property(TARGET leveldb_daemon PROPERTY CXX_STANDARD 14)
set_property(TARGET leveldb_daemon PROPERTY CXX_STANDARD_REQUIRED ON)

add_subdirectory(libsocket)
add_dependencies(leveldb_daemon socket++)

target_link_libraries(leveldb_daemon socket++)
target_link_libraries(leveldb_daemon leveldb)


#shared library for client
project(leveldb_client)

set(LIB_SOURCE_FILES
        ipc/Client.cpp
        logging/Logger.h
        serializer/Serializer.hpp
        include/DB.hpp)

add_library(leveldb_client SHARED ${LIB_SOURCE_FILES})
set_property(TARGET leveldb_client PROPERTY CXX_STANDARD 14)
set_property(TARGET leveldb_client PROPERTY CXX_STANDARD_REQUIRED ON)

add_dependencies(leveldb_client socket++)

target_link_libraries(leveldb_client socket++)
target_link_libraries(leveldb_client protobuf)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)