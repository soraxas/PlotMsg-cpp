#include <chrono>
#include <iostream>
#include <thread>

#include <msg.pb.h>

#include <utility>
#include <zmq.hpp>

namespace Plotly {
// define the static storage
std::unique_ptr<zmq::context_t> static_context;
std::unique_ptr<zmq::socket_t> static_publisher;

// static functions
void initialise_publisher(int sleep_after_bind = 1000,
                          const std::string &addr = "tcp://127.0.0.1:5557") {
  if (static_publisher != nullptr)
    return;
  static_context = std::make_unique<zmq::context_t>();
  static_publisher = std::make_unique<zmq::socket_t>(*static_context, ZMQ_PUB);
  static_publisher->bind(addr);
  if (sleep_after_bind > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_after_bind));
}

class Dictionary;

// easy alias
using DictionaryMsg = PlotlyMsg::Dictionary;
using DictionaryMsgData =
    google::protobuf::Map<std::string, PlotlyMsg::DictItemVal>;

// Helpers
void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        bool value);

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        double value);

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        int value);

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        const std::string &value);

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        const std::vector<double> &value);

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        const std::vector<int> &value);

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        Plotly::Dictionary &value);

// class that represents dict representation in protobuf
class Dictionary {

public:
  Dictionary();

  template <typename T> void add_kwargs(std::string key, const T &value) {
    dictmsg_add_kwargs(m_msg->mutable_data(), key, value);
  }

  void reset();

  DictionaryMsg *release_ptr();

  // variables
  std::unique_ptr<DictionaryMsg> m_msg;
};

class Figure {
public:
  explicit Figure(std::string uuid = "default");

  template <typename T>
  void add_kwargs(const std::string &key, const T &value) {
    dictmsg_add_kwargs(msg.mutable_kwargs()->mutable_data(), key, value);
  }

  template <typename T> void add_kwargs(const std::string &key, T &value) {
    dictmsg_add_kwargs(msg.mutable_kwargs()->mutable_data(), key, value);
  }

  void send(zmq::send_flags send_flags = zmq::send_flags::dontwait);

  void reset();

private:
  zmq::message_t build_zmq_msg();

  // variables
  PlotlyMsg::Figure msg;
  std::string uuid;
};
} // namespace Plotly
