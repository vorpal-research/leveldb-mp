//
// Created by abdullin on 2/29/16.
//

#include <sstream>
#include <string.h>

#include "Client.h"

namespace leveldb_daemon {
namespace ipc {

Client::Client(const std::string &server) : client_(server), log_(LOG_FILE), opened_(true) { }

Client::~Client() {
    if (opened_) {
        close();
    }
}

std::pair<char*, size_t> Client::get(const std::string& key) {
    try {
        client_ << getOneCmd();
        log_.print("Sending: " + getOneCmd());

        auto keySize = intToHexString(key.length());
        client_ << keySize << key;
        log_.print("Sending key with size " + keySize + " and value key " + key);

        std::string dataSize;
        dataSize.resize(WIDTH);
        client_ >> dataSize;
        log_.print("Received data size: " + dataSize);

        auto size = hexStringToInt(dataSize);
        if (size < 0) {
            log_.print("Error: received data with negative length");
        } else if (size == 0) {
            log_.print("Empty input");
            return {nullptr, 0};
        }

        auto data = new char[size];
        memset(data, 0, size);
        client_.rcv(data, size);

        return {data, size};
    } catch (const libsocket::socket_exception& exc) {
        log_.print(exc.mesg);
    }
}

Client::DataArray Client::getAll(const std::string &key) {
    DataArray result;

    try {
        client_ << getAllCmd();
        log_.print("Sending: " + getAllCmd());

        auto keySize = intToHexString(key.length());
        client_ << keySize << key;
        log_.print("Sending key with size " + keySize + " and value key " + key);

        while(true) {
            std::string dataSize;
            dataSize.resize(WIDTH);
            client_ >> dataSize;
            log_.print("Received data size: " + dataSize);
            auto size = hexStringToInt(dataSize);
            if (size < 0) {
                log_.print("Error: received data with negative length");
            }
            auto data = new char[size];
            memset(data, 0, size);
            client_.rcv(data, size);

            if (size == CMD_LENGTH && std::string(data, CMD_LENGTH) == endCmd()) {
                delete []data;
                break;
            }

            result.push_back({data, size});
        }
    } catch (const libsocket::socket_exception& exc) {
        log_.print(exc.mesg);
    }

    return result;
}

bool Client::put(const std::string& key, char* data, size_t size) {
    std::string result;

    try {
        client_ << putCmd();
        log_.print("Sending: " + putCmd());

        auto keySize = intToHexString(key.length());
        client_ << keySize << key;
        log_.print("Sending key with size " + keySize + " and value key " + key);

        auto dataSize = intToHexString(size);
        client_ << dataSize;
        log_.print("Sending data size: " + dataSize);
        client_.snd(data, size);

        result.resize(3);
        client_ >> result;
    } catch (const libsocket::socket_exception& exc) {
        log_.print(exc.mesg);
    }

    return result == successCmd();
}

void Client::close() {
    if (opened_) {
        client_ << endCmd();
        client_.shutdown();
        client_.destroy();
        opened_ = false;
    }
}

std::string Client::intToHexString(const int num, size_t width) {
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

int Client::hexStringToInt(const std::string& str) {
    std::stringstream stream;
    stream << str;
    int num;
    stream >> std::hex >> num;
    return num;
}

}   /* namespace ipc */
}   /* namespace leveldb_daemon */