//
// Created by abdullin on 11/9/16.
//

#include "ipc/Command.h"

namespace leveldb_daemon {
namespace ipc {

const std::string Command::putCmd = "put";
const std::string Command::getAllCmd = "gta";
const std::string Command::getOneCmd = "gto";
const std::string Command::endCmd = "end";
const std::string Command::successCmd = "ok_";
const std::string Command::failCmd = "nok";

}   /* namespace ipc */
}   /* namespace leveldb_daemon */