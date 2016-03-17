cmake_minimum_required(VERSION 3.3)

include_directories(./ ../)
include_directories(/usr/include/leveldb)
include_directories(libsocket/headers)

project(leveldb_daemon)

set(SOURCE_FILES
        main.cpp
        storage/Database.cpp
        ipc/Server.cpp
        ipc/Client.cpp
        logging/Logger.h
        serializer/Serializer.hpp
        DB.hpp)

add_executable(leveldb_daemon ${SOURCE_FILES})
set_property(TARGET leveldb_daemon PROPERTY CXX_STANDARD 14)
set_property(TARGET leveldb_daemon PROPERTY CXX_STANDARD_REQUIRED ON)

add_subdirectory(libsocket)
add_dependencies(leveldb_daemon socket++)

target_link_libraries(leveldb_daemon socket++)
target_link_libraries(leveldb_daemon leveldb)

project(client)

find_package(Protobuf)

file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles})

set(CLIENT_SOURCE_FILES
        client.cpp
        storage/Database.cpp
        ipc/Server.cpp
        ipc/Client.cpp
        logging/Logger.h
        serializer/Serializer.hpp
        DB.hpp
        ${ProtoSources})

add_executable(client ${CLIENT_SOURCE_FILES})
set_property(TARGET client PROPERTY CXX_STANDARD 14)
set_property(TARGET client PROPERTY CXX_STANDARD_REQUIRED ON)

add_dependencies(client socket++)

target_link_libraries(client socket++)
target_link_libraries(client leveldb)
target_link_libraries(client protobuf)