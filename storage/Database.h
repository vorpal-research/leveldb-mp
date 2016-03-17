//
// Created by abdullin on 2/18/16.
//

#ifndef LEVELDB_API_DATABASE_H
#define LEVELDB_API_DATABASE_H

#include <memory>
#include <string>

#include <leveldb/db.h>
#include <leveldb/comparator.h>

#include "logging/Logger.h"

namespace leveldb_daemon {
namespace storage {

class Database {

public:

    using Key = std::string;
    using Value = leveldb::Slice;

    class Iterator;

    Database(const std::string &path);

    bool put(const Key &key, Value value);

    Value get(const Key& key);
    Iterator get(const Key& from, const Key& to);

private:

    std::shared_ptr<leveldb::DB> db_;
    logging::Logger logger_;

};

class Database::Iterator {

    using dbIterator = std::shared_ptr<leveldb::Iterator>;

public:

    Iterator();

    Iterator(leveldb::Iterator *it, const Key &limit);

    void next();

    bool valid() const;

    Value value() const;

private:

    dbIterator it_;
    Key limit_;

};

}   /* namespace storage */
}   /* namespace leveldb_daemon */

#endif //LEVELDB_API_DATABASE_H
