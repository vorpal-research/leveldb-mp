//
// Created by abdullin on 2/26/16.
//

#include <unistd.h>

#include "Server.h"
#include "../util/Util.h"

namespace leveldb_daemon {
namespace ipc {

Server::Server() : db_(DEFAULT_DB_NAME), server_(DEFAULT_SOCKET_NAME), buf_size_(DEFAULT_BUF_SIZE) {
    buffer_ = new char[buf_size_];
    memset(buffer_, 0, buf_size_);
}

Server::Server(const std::string &dbName)
        : db_(dbName), server_(DEFAULT_SOCKET_NAME), buf_size_(DEFAULT_BUF_SIZE) {
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
                log_.print("Received cmd: " + cmd);

                if (cmd == endCmd()) {
                    client->shutdown();
                    delete client;
                    break;
                }

                std::string keySizeStr;
                keySizeStr.resize(WIDTH);
                *client >> keySizeStr;
                auto keySize = util::hexStringToInt(keySizeStr);
                if (keySize > buf_size_) resizeBuffer(keySize);
                std::string key;
                key.resize(keySize);
                *client >> key;
                log_.print("Received key: " + key);

                if (cmd == putCmd()) {
                    std::string dataSizeStr;
                    dataSizeStr.resize(WIDTH);
                    *client >> dataSizeStr;
                    auto dataSize = util::hexStringToInt(dataSizeStr);
                    if (dataSize > buf_size_) resizeBuffer(dataSize);

                    auto totalRecvd = 0;
                    log_.print("Receiving data with size: " + dataSizeStr);
                    while (totalRecvd < dataSize) {
                        auto recvSize = client->rcv(buffer_ + totalRecvd, dataSize - totalRecvd);
                        if (recvSize < 0) {
                            log_.print("Error while receiving");
                            log_.print("Received data:");
                            log_.print(totalRecvd);
                            break;
                        }
                        totalRecvd += recvSize;
                    }
                    log_.print("Received");


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
                        auto&& size = util::intToHexString(it.value().size(), WIDTH);
                        client->snd(size.c_str(), size.length());
                        client->snd(it.value().data(), it.value().size());
                        it.next();
                    }
                    auto&& size = util::intToHexString(CMD_LENGTH, WIDTH);
                    *client << size << endCmd();

                } else if (cmd == getOneCmd()) {
                    auto&& val = db_.get(key);
                    auto&& size = util::intToHexString(val.size(), WIDTH);
                    client->snd(size.c_str(), size.length());
                    if (val.size() > 0) {
                        client->snd(val.data(), val.size());
                    } else {
                        log_.print("Data not found");
                    }

                } else {
                    log_.print("Unknown command from client: " + cmd);
                    log_.print("Disconnecting current client");
                    client->shutdown();
                    delete client;
                    break;
                }
            }
        } catch (const libsocket::socket_exception& ex) {
            log_.print(ex.mesg);
        }
    }
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