//
// Created by abdullin on 2/26/16.
//

#include <unistd.h>

#include "ipc/Command.h"
#include "util/Util.h"

#include "ipc/Server.h"

namespace leveldb_daemon {
namespace ipc {

Server::Server()
        : ObjectLogger(), db_(DEFAULT_DB_NAME), server_(DEFAULT_SOCKET_NAME), buf_size_(DEFAULT_BUF_SIZE) {
    log() << "Creating server" << logging::endl;
    buffer_ = new char[buf_size_];
    memset(buffer_, 0, buf_size_);
}

Server::Server(const std::string &dbName)
        : ObjectLogger(), db_(dbName), server_(DEFAULT_SOCKET_NAME), buf_size_(DEFAULT_BUF_SIZE) {
    log() << "Creating server" << logging::endl;
    buffer_ = new char[buf_size_];
    memset(buffer_, 0, buf_size_);
}

Server::Server(const std::string &dbName, const std::string &socketName)
        : ObjectLogger(), db_(dbName), server_(socketName), buf_size_(DEFAULT_BUF_SIZE) {
    log() << "Creating server" << logging::endl;
    buffer_ = new char[buf_size_];
    memset(buffer_, 0, buf_size_);
}

Server::Server(const std::string &dbName, const std::string &socketName, const size_t bufferSize)
        : ObjectLogger(), db_(dbName), server_(socketName), buf_size_(bufferSize) {
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
            log() << "Received cmd: " << cmd << logging::endl;

            if (cmd == Command::endCmd) {
                client->shutdown();
                delete client;
                break;
            }

            std::string keySizeStr;
            keySizeStr.resize(WIDTH);
            *client >> keySizeStr;
            auto keySize = util::hexStringToInt(keySizeStr);
            if (keySize > buf_size_) reallocBuffer(keySize);
            std::string key;
            key.resize(keySize);
            *client >> key;
            log() << "Received key: " << key << logging::endl;

            if (cmd == Command::putCmd) {
                std::string dataSizeStr;
                dataSizeStr.resize(WIDTH);
                *client >> dataSizeStr;
                auto dataSize = util::hexStringToInt(dataSizeStr);
                if (dataSize > buf_size_) reallocBuffer(dataSize);

                auto totalRecvd = 0;
                log() << "Receiving data with size: " << dataSizeStr << logging::endl;
                while (totalRecvd < dataSize) {
                    auto recvSize = client->rcv(buffer_ + totalRecvd, dataSize - totalRecvd);
                    if (recvSize < 0) {
                        log() << "Error while receiving" << logging::endl;
                        log() << "Received data:" << totalRecvd << logging::endl;
                        break;
                    }
                    totalRecvd += recvSize;
                }
                log() << "Received" << logging::endl;


                leveldb::Slice data(buffer_, dataSize);
                if (not db_.put(key, data)) {
                    log() << "Error while putting data into db with key: " << key << logging::endl;
                    client->snd(Command::failCmd.c_str(), CMD_LENGTH);
                } else {
                    client->snd(Command::successCmd.c_str(), CMD_LENGTH);
                }
                memset(buffer_, 0, dataSize);

            } else if (cmd == Command::getAllCmd) {
                auto&& it = db_.get(key, key);
                while (it.valid()) {
                    auto&& size = util::intToHexString(it.value().size(), WIDTH);
                    client->snd(size.c_str(), size.length());
                    client->snd(it.value().data(), it.value().size());
                    it.next();
                }
                auto&& size = util::intToHexString(CMD_LENGTH, WIDTH);
                *client << size << Command::endCmd;

            } else if (cmd == Command::getOneCmd) {
                auto&& val = db_.get(key);
                auto&& size = util::intToHexString(val.size(), WIDTH);
                client->snd(size.c_str(), size.length());
                if (val.size() > 0) {
                    client->snd(val.data(), val.size());
                } else {
                    log() << "Data not found" << logging::endl;
                }

            } else {
                log() << "Unknown command from client: " << cmd << logging::endl;
                log() << "Disconnecting current client" << logging::endl;
                client->shutdown();
                delete client;
                break;
            }
        }
    } catch (const libsocket::socket_exception& ex) {
        log() << ex.mesg << logging::endl;
    }

    }
}

void Server::reallocBuffer(size_t size) {
    if (size > 0) {
        char* newBuffer = new char[size];
        memcpy(newBuffer, buffer_, buf_size_);
        delete buffer_;
        buffer_ = newBuffer;
        buf_size_ = size;
    } else {
        log() << "Error: trying to initialize buffer with negative length" << logging::endl;
    }
}

const std::string Server::DEFAULT_DB_NAME = "/tmp/leveldb-testbase";
const std::string Server::DEFAULT_SOCKET_NAME = "/tmp/leveldb-test-server-socket.soc";

}   /* namespace ipc */
}   /* namespace leveldb_daemon */