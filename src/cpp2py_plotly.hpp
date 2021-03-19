#include <chrono>
#include <iostream>
#include <thread>

#include <build/msg.pb.h>

#include <utility>
#include <zmq.hpp>

namespace Plotly {
// define the static storage
std::string addr = "tcp://127.0.0.1:5557";
std::unique_ptr<zmq::context_t> static_context;
std::unique_ptr<zmq::socket_t> static_publisher;

// static functions
void set_addr(const std::string &new_address) { addr = new_address; }
void initialise_publisher(int sleep_after_bind = 1000) {
  if (static_publisher != nullptr)
    return;
  static_context = std::make_unique<zmq::context_t>();
  static_publisher = std::make_unique<zmq::socket_t>(*static_context, ZMQ_PUB);
  static_publisher->bind(addr);
  if (sleep_after_bind > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_after_bind));
}

// easy alias
using DictionaryMsg = PlotlyMsg::Dictionary;
using DictionaryMsgData =
    google::protobuf::Map<std::string, PlotlyMsg::DictItemVal>;

//////////////////////////////////////////////////////////////////////////
// helpers
PlotlyMsg::SeriesD *vec_to_allocated_seriesD(std::vector<double> value) {
  google::protobuf::RepeatedField<double> data(value.begin(), value.end());
  auto *series = new PlotlyMsg::SeriesD();
  series->mutable_data()->Swap(&data);
  return series;
}

PlotlyMsg::SeriesI *vec_to_allocated_seriesI(std::vector<int> value) {
  google::protobuf::RepeatedField<int> data(value.begin(), value.end());
  auto *series = new PlotlyMsg::SeriesI();
  series->mutable_data()->Swap(&data);
  return series;
}

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        bool value) {
  (*msg)[key].set_bool_(value);
}

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        double value) {
  (*msg)[key].set_double_(value);
}

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        int value) {
  (*msg)[key].set_int_(value);
}

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        const std::string &value) {
  (*msg)[key].set_string(value);
}

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        const std::vector<double> &value) {
  (*msg)[key].set_allocated_series_d(vec_to_allocated_seriesD(value));
}

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        const std::vector<int> &value) {
  (*msg)[key].set_allocated_series_i(vec_to_allocated_seriesI(value));
}
//////////////////////////////////////////////////////////////////////////

// class that represents dict representation in protobuf
class Dictionary {
public:
  Dictionary() { reset(); }

  template <typename T> void add_kwargs(std::string key, const T &value) {
    dictmsg_add_kwargs(msg->mutable_data(), key, value);
  }

  void reset() { msg = std::make_unique<DictionaryMsg>(); }

  DictionaryMsg *release_ptr() { return msg.release(); }

  std::unique_ptr<DictionaryMsg> msg;
};

void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
                        Plotly::Dictionary &value) {
  (*msg)[key].set_allocated_dict(value.release_ptr());
}

class Figure {
public:
  explicit Figure(std::string uuid = "default") : uuid(std::move(uuid)) {
    reset();
  }

  template <typename T>
  void add_kwargs(const std::string &key, const T &value) {
    dictmsg_add_kwargs(msg.mutable_kwargs()->mutable_data(), key, value);
  }

  template <typename T> void add_kwargs(const std::string &key, T &value) {
    dictmsg_add_kwargs(msg.mutable_kwargs()->mutable_data(), key, value);
  }

  void send(zmq::send_flags send_flags = zmq::send_flags::dontwait) {
    initialise_publisher();
    static_publisher->send(std::move(build_zmq_msg()), send_flags);
  }

  void reset() {
    msg.Clear();
    msg.set_uuid(uuid);
  }

private:
  zmq::message_t build_zmq_msg() {
    // serial protobuf to string
    std::string encoded_msg;
    msg.SerializeToString(&encoded_msg);
    // memcpy to a zmq msg
    zmq::message_t zmq_msg(encoded_msg.size());
    memcpy((void *)zmq_msg.data(), encoded_msg.c_str(), encoded_msg.size());
    return zmq_msg;
  }

  PlotlyMsg::Figure msg;
  std::string uuid;
};
} // namespace Plotly
