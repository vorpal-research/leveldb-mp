//
// Created by abdullin on 2/29/16.
//

#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#include <vector>

#include "unixclientstream.hpp"

#include "../logging/Logger.h"

namespace leveldb_daemon {
namespace ipc {

class Client {

public:

    using DataArray = std::vector< std::pair<char*, size_t> >;

    static const size_t WIDTH = 8;
    static const size_t CMD_LENGTH = 3;
    const std::string LOG_FILE = "/tmp/leveldb-api-client.log";

    Client(const std::string& server);
    ~Client();

    std::pair<char*, size_t> get(const std::string& key);
    DataArray getAll(const std::string& key);
    bool put(const std::string& key, char* data, size_t size);
    void close();

private:

    std::string putCmd() const      { return "put"; }
    std::string getAllCmd() const   { return "gta"; }
    std::string getOneCmd() const   { return "gto"; }
    std::string endCmd() const      { return "end"; }
    std::string successCmd() const  { return "ok_"; }
    std::string failCmd() const     { return "nok"; }

    std::string intToHexString(const int num, const size_t width = WIDTH);
    int hexStringToInt(const std::string& str);

private:

    libsocket::unix_stream_client client_;
    logging::Logger log_;

};

}   /* namespace ipc */
}   /* namespace leveldb_daemon */

#endif //CLIENT_CLIENT_H
