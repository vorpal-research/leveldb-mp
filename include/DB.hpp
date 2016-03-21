#include <memory>

#include "ipc/Client.h"
#include "ipc/Server.h"
#include "serializer/Serializer.hpp"


namespace leveldb_daemon {

class DB {

public:

    using Ptr = std::shared_ptr<DB>;

    static DB::Ptr getInstance() {
        if (not instance) instance.reset( new DB{} );
        return instance;
    }

    void setPath(const std::string& db_path) {
        db_path_ = db_path;
    }

    template<class T>
    bool write(const std::string& key, const T& obj) {
        auto byteStream = serializer::serializer<T>::serialize(obj);
        ipc::Client client(db_path_);
        return client.put(key, byteStream.array.get(),byteStream.size);
    };

    template<class ResT, class Context>
    auto read(const std::string& key, Context& ctx) -> decltype(auto) {
        ipc::Client client(db_path_);
        auto&& serializedData = client.get(key);
        std::shared_ptr<char> ptr(serializedData.first);
        serializer::Buffer value{ptr, serializedData.second};
        return (value.size < 1) ?
               serializer::deserializer<ResT, serializer::Buffer, Context>::notFound() :
               serializer::deserializer<ResT, serializer::Buffer, Context>::deserialize(value, ctx);
    };

    template<class ResT>
    auto read(const std::string& key) -> decltype(auto) {
        ipc::Client client(db_path_);
        auto&& serializedData = client.get(key);
        std::shared_ptr<char> ptr(serializedData.first);
        serializer::Buffer value{ptr, serializedData.second};
        return (value.size < 1) ?
               serializer::deserializer<ResT, serializer::Buffer>::notFound() :
               serializer::deserializer<ResT, serializer::Buffer>::deserialize(value);
    };

    template<class ResT, class Context>
    auto readAll(const std::string& key, Context& ctx) -> decltype(auto) {
        ipc::Client client(db_path_);
        auto&& serializedData = client.getAll(key);
        std::vector<ResT> result;
        for (auto&& it: serializedData) {
            std::shared_ptr<char> ptr(it.first);
            serializer::Buffer value{ptr, it.second};
            result.push_back(serializer::deserializer<ResT, serializer::Buffer, Context>::deserialize(value, ctx));
        }
        return result;
    };


    template<class ResT>
    auto readAll(const std::string& key) -> decltype(auto) {
        ipc::Client client(db_path_);
        auto&& serializedData = client.getAll(key);
        std::vector<ResT> result;
        for (auto&& it: serializedData) {
            std::shared_ptr<char> ptr(it.first);
            serializer::Buffer value{ptr, it.second};
            result.push_back(serializer::deserializer<ResT, serializer::Buffer>::deserialize(value));
        }
        return result;
    };

private:

    DB() {}
    DB(const std::string& db_path): db_path_(db_path) {}
    DB( const DB& ) = delete;
    DB& operator=( DB& ) = delete;

    std::string db_path_;
    static DB::Ptr instance;

};

}   /* namespace leveldb_daemon */