#ifndef DYNAMICAPP_DB_HPP
#define DYNAMICAPP_DB_HPP

#include <iostream>
#include <memory>

namespace leveldb_daemon {
namespace serializer {

struct Buffer{
    std::shared_ptr<char> array;
    size_t size;
};

template<class T, class ResultT = char*> struct serializer;

template<class ResultT, class SerializedT, class Context=std::string> struct deserializer;

template<class T, class ResultT>
struct serializer {
    static Buffer serialize(const T &s) {
        return T::serialize(s);
    }
};

template<class ResultT, class SerializedT, class Context>
struct deserializer {

    static auto deserialize(const SerializedT& s,Context& ctx) -> decltype(auto) {
        return ResultT::deserialize(s,ctx);
    }
    static auto deserialize(const SerializedT& s) -> decltype(auto) {
        return ResultT::deserialize(s);
    }

    static auto notFound() -> decltype(auto) {
        return ResultT::notFound();
    }

};

}   /* namespace serializer */
}   /* namespace leveldb_daemon */

#endif