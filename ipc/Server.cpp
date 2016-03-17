//
// Created by abdullin on 2/26/16.
//

#include <sstream>
#include <unistd.h>

#include "Server.h"

namespace leveldb_daemon {
namespace ipc {

Server::Server() : db_(DEFAULT_DB_NAME), server_(DEFAULT_SOCKET_NAME), buf_size_(DEFAULT_BUF_SIZE) {
    buffer_ = new char[buf_size_];
    memset(buffer_, 0, buf_size_);
}

Server::Server(const std::string &dbName, const std::string &socketName)
        : db_(dbName), server_(socketName), buf_size_(DEFAULT_BUF_SIZE) {
    buffer_ = new char[buf_size_];
    memset(buffer_, 0, buf_size_);
}

Server::Server(const std::string &dbName, const std::string &socketName, const size_t bufferSize)
        : db_(dbName), server_(socketName), buf_size_(bufferSize) {
    buffer_ = new char[buf_size_];
    memset(buffer_, 0, buf_size_);
}

Server::~Server() {
    destroy();
}

void Server::destroy() {
    server_.destroy();
    delete buffer_;
}

int Server::work() {
    while (true) {

        try {
            libsocket::unix_stream_client* client;
            client = server_.accept();

            while (true) {
                std::string cmd;
                cmd.resize(CMD_LENGTH);
                *client >> cmd;

                if (cmd == endCmd()) {
                    client->shutdown();
                    delete client;
                    break;
                }

                std::string keySizeStr;
                keySizeStr.resize(WIDTH);
                *client >> keySizeStr;
                auto keySize = hexStringToInt(keySizeStr);
                if (keySize > buf_size_) resizeBuffer(keySize);
                std::string key;
                key.resize(keySize);
                *client >> key;

                if (cmd == putCmd()) {
                    std::string dataSizeStr;
                    dataSizeStr.resize(WIDTH);
                    *client >> dataSizeStr;
                    auto dataSize = hexStringToInt(dataSizeStr);
                    if (dataSize > buf_size_) resizeBuffer(dataSize);
                    auto recvSize = client->rcv(buffer_, dataSize);
                    if (recvSize > buf_size_) resizeBuffer(recvSize);

                    leveldb::Slice data(buffer_, dataSize);
                    if (not db_.put(key, data)) {
                        log_.print("Error while putting data into db with key: " + key);
                        client->snd(failCmd().c_str(), CMD_LENGTH);
                    } else {
                        client->snd(successCmd().c_str(), CMD_LENGTH);
                    }
                    memset(buffer_, 0, dataSize);

                } else if (cmd == getAllCmd()) {
                    auto&& it = db_.get(key, key);
                    while (it.valid()) {
                        auto&& size = intToHexString(it.value().size());
                        client->snd(size.c_str(), size.length());
                        client->snd(it.value().data(), it.value().size());
                        it.next();
                    }
                    auto&& size = intToHexString(CMD_LENGTH);
                    *client << size << endCmd();
                } else if (cmd == getOneCmd()) {
                    auto&& val = db_.get(key);
                    auto&& size = intToHexString(val.size());
                    client->snd(size.c_str(), size.length());
                    client->snd(val.data(), val.size());
                }
            }
        } catch (const libsocket::socket_exception& ex) {
            log_.print(ex.mesg);
        }
    }
}

std::string Server::intToHexString(const int num, const size_t width) {
    std::string res;
    std::stringstream stream;
    stream << std::hex << num;
    stream >> res;
    if (res.length() < width) {
        std::string nulls(width - res.length(), '0');
        res.insert(0, nulls);
    } else if (res.length() > width) {
        log_.print("Error: size of data is too big");
        res = std::string(width, '0');
    }
    return res;
}

int Server::hexStringToInt(const std::string& str) {
    std::stringstream stream;
    stream << str;
    int num;
    stream >> std::hex >> num;
    return num;
}

void Server::resizeBuffer(size_t size) {
    if (size > 0) {
        char* newBuffer = new char[size];
        memcpy(newBuffer, buffer_, buf_size_);
        delete buffer_;
        buffer_ = newBuffer;
        buf_size_ = size;
    } else {
        log_.print("Error: trying to initialize buffer with negative length");
    }
}

const std::string Server::DEFAULT_DB_NAME = "/tmp/leveldb-testbase";
const std::string Server::DEFAULT_SOCKET_NAME = "/tmp/leveldb-test-server-socket.soc";

}   /* namespace ipc */
}   /* namespace leveldb_daemon */