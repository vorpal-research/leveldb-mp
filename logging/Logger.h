//
// Created by abdullin on 2/19/16.
//

#ifndef LEVELDB_API_LOGGER_H
#define LEVELDB_API_LOGGER_H

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <bits/shared_ptr.h>

namespace leveldb_daemon {
namespace logging {

static std::string endl = "\n";

static std::ostream& show_time(std::ostream& out) {
    time_t time_val;
    time(&time_val);

    struct tm *local_time;
    local_time = localtime(&time_val);

    auto day = local_time->tm_mday;
    auto month = local_time->tm_mon + 1;
    auto year = local_time->tm_year + 1900;
    auto hour = local_time->tm_hour;
    auto minute = local_time->tm_min;
    auto second = local_time->tm_sec;

    out << "[" << year << "-" << month << "-" << day << " " << hour << ":" << minute << ":" << second << "] ";
    return out;
}

class Logger {

public:

    Logger() : Logger("/tmp/leveldb-api.log") { }

    Logger(const std::string& logfile) {
        logstream.open(logfile, std::ios::app);
    }

    ~Logger() {
        logstream.close();
    }

    template<typename T>
    Logger& operator<<(const T& data) {
        logstream << data;
    }

private:

    std::ofstream logstream;

};

}   /* namespace logging */
}   /* namespace leveldb_daemon */

#endif //LEVELDB_API_LOGGER_H
