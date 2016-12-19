//
// Created by abdullin on 12/16/16.
//

#ifndef LEVELDB_CLIENT_CONFIG_H
#define LEVELDB_CLIENT_CONFIG_H

namespace leveldb_mp {
namespace config {

static const std::string DAEMON_FILE_PATH = "/tmp/leveldb_daemon_";
static const std::string LOG_FILE = "/tmp/leveldb_mp.log";
static const std::string OUTPUT_FILE_PATH = "/tmp/";
static const std::string SOCKET_POSTFIX = ".soc";


static const std::string DEFAULT_DB_NAME = "/tmp/leveldb_testbase";
static const std::string DEFAULT_SOCKET_NAME = "/tmp/leveldb_test_server_socket.soc";

}   /* namespace config */
}   /* namespace leveldb_mp */

#endif //LEVELDB_CLIENT_CONFIG_H
