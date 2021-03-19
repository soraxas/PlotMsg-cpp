#include "cpp2py_plotly.hpp"

namespace Plotly {

////////////////////////////////////////
// implementation of Dictionary
////////////////////////////////////////
Dictionary::Dictionary() { reset(); }

template <typename T>
void Dictionary::add_kwargs(std::string key, const T &value) {
  dictmsg_add_kwargs(m_msg->mutable_data(), key, value);
}
void Dictionary::reset() { m_msg = std::make_unique<DictionaryMsg>(); }

DictionaryMsg *Dictionary::release_ptr() { return m_msg.release(); }

////////////////////////////////////////
// implementation of Plotly Figure
////////////////////////////////////////
Figure::Figure(std::string uuid) : uuid(std::move(uuid)) { reset(); }

template <typename T>
void Figure::add_kwargs(const std::string &key, const T &value) {
  dictmsg_add_kwargs(msg.mutable_kwargs()->mutable_data(), key, value);
}

template <typename T>
void Figure::add_kwargs(const std::string &key, T &value) {
  dictmsg_add_kwargs(msg.mutable_kwargs()->mutable_data(), key, value);
}

void Figure::send(zmq::send_flags send_flags) {
  initialise_publisher();
  static_publisher->send(build_zmq_msg(), send_flags);
}

void Figure::reset() {
  msg.Clear();
  msg.set_uuid(uuid);
}
zmq::message_t Figure::build_zmq_msg() {
  // serial protobuf to string
  std::string encoded_msg;
  msg.SerializeToString(&encoded_msg);
  // memcpy to a zmq m_msg
  zmq::message_t zmq_msg(encoded_msg.size());
  memcpy((void *)zmq_msg.data(), encoded_msg.c_str(), encoded_msg.size());
  return zmq_msg;
}

////////////////////////////////////////
// Helpers
////////////////////////////////////////
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
}; // namespace Plotly