//
// Created by abdullin on 2/26/16.
//

#ifndef LEVELDB_API_SERVER_H
#define LEVELDB_API_SERVER_H

#include <unixserverstream.hpp>

#include "logging/Logger.h"
#include "storage/Database.h"

namespace leveldb_daemon {
namespace ipc {

class Server: public logging::ObjectLogger {

public:

    static const std::string DEFAULT_DB_NAME;
    static const std::string DEFAULT_SOCKET_NAME;
    static const size_t DEFAULT_BUF_SIZE = 64 * 1024;

    static const size_t WIDTH = 8;
    static const size_t CMD_LENGTH = 3;

    Server();
    Server(const std::string& dbName);
    Server(const std::string& dbName, const std::string& socketName);
    Server(const std::string& dbName, const std::string& socketName, const size_t bufferSize);
    ~Server();

    int work();
    void destroy();

private:

    void reallocBuffer(size_t size);

private:

    storage::Database db_;
    libsocket::unix_stream_server server_;
    char* buffer_;
    size_t buf_size_;

};

}   /* namespace ipc */
}   /* namespace leveldb_daemon */

#endif //LEVELDB_API_SERVER_H
