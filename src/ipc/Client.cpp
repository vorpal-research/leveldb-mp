//
// Created by abdullin on 2/29/16.
//

#include <string.h>

#include "ipc/Command.h"
#include "util/Util.h"

#include "ipc/Client.h"

namespace leveldb_daemon {
namespace ipc {

Client::Client() : ObjectLogger(LOG_FILE), opened_(false) { }
Client::Client(const std::string &server) : ObjectLogger(LOG_FILE), client_(server), opened_(true) { }

Client::~Client() {
    close();
}

void Client::connect(const std::string& server) {
    client_.connect(server);
    opened_ = true;
}

std::pair<char*, size_t> Client::get(const std::string& key) {
    try {
        client_ << Command::getOneCmd;
        log() << "Sending: " << Command::getOneCmd << logging::endl;

        auto keySize = util::intToHexString(key.length(), WIDTH);
        client_ << keySize << key;
        log() << "Sending key: " << key << logging::endl;

        std::string dataSize;
        dataSize.resize(WIDTH);
        client_ >> dataSize;

        auto size = util::hexStringToInt(dataSize);
        if (size < 0) {
            log() << "Error: received data with negative length" << logging::endl;
        } else if (size == 0) {
            log() << "Empty input" << logging::endl;
            return {nullptr, 0};
        }

        auto data = new char[size];
        memset(data, 0, size);
        receiveData(data, size);

        return {data, size};
    } catch (const libsocket::socket_exception& exc) {
        log() << exc.mesg << logging::endl;
    }
    return {nullptr, 0};
}

Client::DataArray Client::getAll(const std::string &key) {
    DataArray result;

    try {
        client_ << Command::getAllCmd;
        log() << "Sending: " << Command::getAllCmd << logging::endl;

        auto keySize = util::intToHexString(key.length(), WIDTH);
        client_ << keySize << key;
        log() << "Sending key: " << key << logging::endl;

        while(true) {
            std::string dataSize;
            dataSize.resize(WIDTH);
            client_ >> dataSize;
            auto size = util::hexStringToInt(dataSize);
            if (size < 0) {
                log() << "Error: received data with negative length" << logging::endl;
            } else if (size == 0) {
                log() << "Empty input" << logging::endl;
                continue;
            }

            auto data = new char[size];
            memset(data, 0, size);

            receiveData(data, size);

            if (size == CMD_LENGTH && std::string(data, CMD_LENGTH) == Command::endCmd) {
                log() << "Received: " << Command::endCmd << logging::endl;
                delete []data;
                break;
            }

            result.push_back({data, size});
        }
    } catch (const libsocket::socket_exception& exc) {
        log() << exc.mesg << logging::endl;
    }

    return result;
}

bool Client::put(const std::string& key, char* data, size_t size) {
    std::string result;

    try {
        client_ << Command::putCmd;
        log() << "Sending: " << Command::putCmd << logging::endl;

        auto keySize = util::intToHexString(key.length(), WIDTH);
        client_ << keySize << key;
        log() << "Sending key: " << key << logging::endl;

        auto dataSize = util::intToHexString(size, WIDTH);
        client_ << dataSize;
        log() << "Sending data size: " << dataSize << logging::endl;
        if (size > 0) client_.snd(data, size);

        result.resize(3);
        client_ >> result;
    } catch (const libsocket::socket_exception& exc) {
        log() << exc.mesg << logging::endl;
    }

    return result == Command::successCmd;
}

void Client::close() {
    if (opened_) {
        client_ << Command::endCmd;
        client_.shutdown();
        client_.destroy();

        log() << "Client destroyed" << logging::endl;

        opened_ = false;
    }
}

bool Client::receiveData(char *buffer, size_t size) {
    auto totalRecvd = 0;
    log() << "Receiving data with size:" << size << logging::endl;
    while (totalRecvd < size) {
        auto recvd = client_.rcv(buffer + totalRecvd, size - totalRecvd);
        if (recvd < 0) {
            log() << "Error while receiving" << logging::endl;
            return false;
        }
        totalRecvd += recvd;
    }
    log() << "Receiving end" << logging::endl;
    return true;
}

}   /* namespace ipc */
}   /* namespace leveldb_daemon */