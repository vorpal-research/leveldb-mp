#include <memory>

#include "../ipc/Client.h"
#include "../ipc/Server.h"
#include "../serializer/Serializer.hpp"


namespace leveldb_daemon {

class DB {

public:

    using Ptr = std::shared_ptr<DB>;
    static const std::string DAEMON_FILE_PATH;

    static DB::Ptr getInstance() {
        static DB::Ptr instance_ = DB::Ptr(new DB{});
        return instance_;
    }

    static bool isDaemonStarted() {
        std::ifstream file(DAEMON_FILE_PATH);
        auto&& exists = file.good();
        file.close();
        return exists;
    }

    void setSocket(const std::string& socket_name) {
        socket_name_ = socket_name;
    }

    template<class T>
    bool write(const std::string& key, const T& obj) {
        auto byteStream = serializer::serializer<T>::serialize(obj);
        ipc::Client client(socket_name_);
        return client.put(key, byteStream.array.get(),byteStream.size);
    };

    template<class ResT, class Context>
    auto read(const std::string& key, Context& ctx) -> decltype(auto) {
        ipc::Client client(socket_name_);
        auto&& serializedData = client.get(key);
        std::shared_ptr<char> ptr(serializedData.first);
        serializer::Buffer value{ptr, serializedData.second};
        return (value.size < 1) ?
               serializer::deserializer<ResT, serializer::Buffer, Context>::notFound() :
               serializer::deserializer<ResT, serializer::Buffer, Context>::deserialize(value, ctx);
    };

    template<class ResT>
    auto read(const std::string& key) -> decltype(auto) {
        ipc::Client client(socket_name_);
        auto&& serializedData = client.get(key);
        std::shared_ptr<char> ptr(serializedData.first);
        serializer::Buffer value {ptr, serializedData.second};
        return (value.size < 1) ?
               serializer::deserializer<ResT, serializer::Buffer>::notFound() :
               serializer::deserializer<ResT, serializer::Buffer>::deserialize(value);
    };

    template<class ResT, class Context>
    auto readAll(const std::string& key, Context& ctx) -> decltype(auto) {
        ipc::Client client(socket_name_);
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
        ipc::Client client(socket_name_);
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
    DB(const std::string& db_path): socket_name_(db_path) {}
    DB( const DB& ) = delete;
    DB& operator=( DB& ) = delete;

    std::string socket_name_;

};

}   /* namespace leveldb_daemon */