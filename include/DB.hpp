#ifndef LEVELDB_API_DB_HPP
#define LEVELDB_API_DB_HPP

#include <memory>

#include "ipc/Client.h"
#include "ipc/Server.h"
#include "Serializer.hpp"
#include "util/Util.h"


namespace leveldb_mp {

class DB {

public:

    using Ptr = std::shared_ptr<DB>;

    static DB::Ptr getInstance() {
        static DB::Ptr instance_ = DB::Ptr(new DB{});
        return instance_;
    }

    static bool isDaemonStarted(const std::string& db_name) {
        return util::isFileExists(config::DAEMON_FILE_PATH + db_name);
    }

    void connect(const std::string& db_name) {
        socket_name_ = config::OUTPUT_FILE_PATH + db_name + config::SOCKET_POSTFIX;
    }

    void lock() {
        locked_ = true;
        client_.connect(socket_name_);
    }

    void unlock() {
        client_.close();
        locked_ = false;
    }

#define EXEC(inst) if (not locked_) client_.connect(socket_name_); \
    inst; \
    if (not locked_) client_.close();

    template<class T>
    bool write(const std::string& key, const T& obj) {
        auto byteStream = serializer::serializer<T>::serialize(obj);
        EXEC(auto&& result = client_.put(key, byteStream.array.get(), byteStream.size));
        return result;
    };

    template<class ResT, class Context>
    auto read(const std::string& key, Context& ctx) -> decltype(auto) {
        EXEC(auto&& serializedData = client_.get(key));
        if (not serializedData.first)
            return serializer::deserializer<ResT, serializer::Buffer, Context>::notFound();
        std::shared_ptr<char> ptr(serializedData.first);
        serializer::Buffer value{ptr, serializedData.second};
        return serializer::deserializer<ResT, serializer::Buffer, Context>::deserialize(value, ctx);
    };

    template<class ResT>
    auto read(const std::string& key) -> decltype(auto) {
        EXEC(auto&& serializedData = client_.get(key));
        if (not serializedData.first)
            return serializer::deserializer<ResT, serializer::Buffer>::notFound();
        std::shared_ptr<char> ptr(serializedData.first);
        serializer::Buffer value {ptr, serializedData.second};
        return serializer::deserializer<ResT, serializer::Buffer>::deserialize(value);
    };

    template<class ResT, class Context>
    auto readAll(const std::string& key, Context& ctx) -> decltype(auto) {
        EXEC(auto&& serializedData = client_.getAll(key));
        std::vector<decltype(serializer::deserializer<ResT, serializer::Buffer, Context>::deserialize(
                serializer::Buffer{std::shared_ptr<char>(serializedData[0].first), serializedData[0].second}, ctx))> result;
        for (auto&& it: serializedData) {
            std::shared_ptr<char> ptr(it.first);
            serializer::Buffer value{ptr, it.second};
            result.push_back(serializer::deserializer<ResT, serializer::Buffer, Context>::deserialize(value, ctx));
        }
        return result;
    };


    template<class ResT>
    auto readAll(const std::string& key) -> decltype(auto) {
        EXEC(auto&& serializedData = client_.getAll(key));
        std::vector<decltype(serializer::deserializer<ResT, serializer::Buffer>::deserialize(
                serializer::Buffer{std::shared_ptr<char>(serializedData[0].first), serializedData[0].second}))> result;
        for (auto&& it: serializedData) {
            std::shared_ptr<char> ptr(it.first);
            serializer::Buffer value{ptr, it.second};
            result.push_back(serializer::deserializer<ResT, serializer::Buffer>::deserialize(value));
        }
        return result;
    };

#undef EXEC

private:

    DB() {}
    DB(const std::string& db_path): socket_name_(db_path), locked_(false) {}
    DB( const DB& ) = delete;
    DB& operator=( DB& ) = delete;

    std::string socket_name_;
    ipc::Client client_;
    bool locked_;

};

}   /* namespace leveldb_mp */

#endif