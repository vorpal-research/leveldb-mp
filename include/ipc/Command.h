//
// Created by abdullin on 11/9/16.
//

#ifndef LEVELDB_CLIENT_COMMAND_H
#define LEVELDB_CLIENT_COMMAND_H

#include <string>

namespace leveldb_daemon {
namespace ipc {

class Command {

public:

    static const std::string putCmd;
    static const std::string getAllCmd;
    static const std::string getOneCmd;
    static const std::string endCmd;
    static const std::string successCmd;
    static const std::string failCmd;

};

}   /* namespace ipc */
}   /* namespace leveldb_daemon */

#endif //LEVELDB_CLIENT_COMMAND_H
