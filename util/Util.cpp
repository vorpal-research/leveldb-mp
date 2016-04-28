//
// Created by abdullin on 4/28/16.
//

#include "Util.h"

namespace leveldb_daemon {
namespace util {

bool isFileExists(const std::string& path) {
    std::ifstream file(path);
    auto&& exists = file.good();
    file.close();
    return exists;
}

std::string intToHexString(const int num, const size_t width) {
    logging::Logger log;
    std::string res;
    std::stringstream stream;
    stream << std::hex << num;
    stream >> res;
    if (res.length() < width) {
        std::string nulls(width - res.length(), '0');
        res.insert(0, nulls);
    } else if (res.length() > width) {
        log.print("Error: size of data is too big");
        res = std::string(width, '0');
    }
    return res;
}

int hexStringToInt(const std::string& str) {
    std::stringstream stream;
    stream << str;
    int num;
    stream >> std::hex >> num;
    return num;
}

}   /* namespace utl */
}   /* namespace leveldb_daemon */
