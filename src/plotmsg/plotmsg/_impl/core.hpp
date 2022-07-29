#pragma once

#include <msg.pb.h>
#include <zmq.hpp>

#define PLOTMSG_DEFAULT_ADDR "tcp://127.0.0.1:5557"

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
// C++17 specific
#define INLINE inline
#else
#define INLINE
#endif

namespace PlotMsg
{
    using NullValueType = PlotMsgProto::NullValue;
    NullValueType NullValue = PlotMsgProto::NULL_VALUE;

    // easy alias
    using DictionaryMsgData = google::protobuf::Map<std::string, PlotMsgProto::DictItemValMsg>;

    // define the static storage
    INLINE std::unique_ptr<zmq::context_t> static_context;
    INLINE std::unique_ptr<zmq::socket_t> static_publisher;

    // static functions
    void initialise_publisher(
        int sleep_after_bind = 1000, const std::string &addr = PLOTMSG_DEFAULT_ADDR
    );

    std::ostream &operator<<(std::ostream &out, DictionaryMsgData const &dict);

}  // namespace PlotMsg
