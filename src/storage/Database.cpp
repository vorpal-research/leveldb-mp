//
// Created by abdullin on 2/18/16.
//

#include "storage/Database.h"

using namespace leveldb;

namespace leveldb_daemon {
namespace storage {

Database::Database(const std::string &path) : ObjectLogger() {
    Options options;
    options.create_if_missing = true;
    options.compression = kNoCompression;

    DB *db;
    log() << "Open database:" << path << logging::endl;
    auto&& status = DB::Open(options, path, &db);
    if (not status.ok()) {
        log() << status.ToString() << logging::endl;
        exit(1);
    }
    db_.reset(db);
}

bool Database::put(const std::string &key, Value value) {
    auto&& status = db_->Put(WriteOptions(), Slice(key), value);

    if (not status.ok()) {
        log() << status.ToString() << logging::endl;
    }

    return status.ok();
}

Database::Value Database::get(const Key &key) {
    auto&& it = db_->NewIterator(ReadOptions());

    it->Seek(Slice(key));

    if (it->Valid() && it->key().ToString() == key)
        return it->value();
    else
        return Value("", 0);
}

Database::Iterator Database::get(const Key& from, const Key& to) {
    auto&& it = db_->NewIterator(ReadOptions());

    it->Seek(Slice(from));
    return Database::Iterator(it, to);
}

Database::Iterator::Iterator() : it_(nullptr) { }

Database::Iterator::Iterator(leveldb::Iterator *it, const Key &limit) : it_(it), limit_(limit) { }

void Database::Iterator::next() {
    if (it_ && it_->Valid()) {
        it_->Next();
    }
}

bool Database::Iterator::valid() const {
    if (not it_) return false;

    return it_->Valid() && it_->key().starts_with(Slice(limit_));
}

Database::Value Database::Iterator::value() const {
    if (not valid()) return Value();
    else return it_->value();
}

}   /* namespace storage */
}   /* namespace leveldb_daemon */