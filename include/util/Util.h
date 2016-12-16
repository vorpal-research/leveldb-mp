//
// Created by abdullin on 4/28/16.
//

#ifndef LEVELDB_MP_UTIL_H
#define LEVELDB_MP_UTIL_H

#include <fstream>
#include <sstream>

#include "logging/Logger.h"

namespace leveldb_mp {
namespace util {

bool isFileExists(const std::string& path);

std::string intToHexString(const int num, const size_t width);
int hexStringToInt(const std::string& str);

}   /* namespace utl */
}   /* namespace leveldb_mp */

#endif //LEVELDB_MP_UTIL_H
