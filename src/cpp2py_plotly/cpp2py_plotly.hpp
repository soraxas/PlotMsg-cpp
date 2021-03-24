#pragma once

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

// easy alias
using DictionaryMsg = PlotlyMsg::Dictionary;
using DictionaryMsgData =
    google::protobuf::Map<std::string, PlotlyMsg::DictItemVal>;

std::ostream &operator<<(std::ostream &out, DictionaryMsgData const &dict);

// forward declare
class Dictionary;

////////////////////////////////////////
// Helpers
////////////////////////////////////////

PlotlyMsg::SeriesD *vec_to_allocated_seriesD(std::vector<double> value);

PlotlyMsg::SeriesI *vec_to_allocated_seriesI(std::vector<int> value);

// helper function to assign given PlotlyMsg::DictItemVal with T value
void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, bool value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, double value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, int value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, const char *value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, std::string &value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      const std::vector<double> &value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      const std::vector<int> &value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      Plotly::Dictionary &value);

// r-value, uses l-value definition
void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      Plotly::Dictionary &&value);

////////////////////////////////////////

// class that represents dict representation in protobuf
class Dictionary {

public:
  class DictionaryItemPair {
  public:
    template <typename T> DictionaryItemPair(const std::string &key, T value) {
      m_key = key;
      Plotly::_set_DictItemVal(m_item_val, std::forward<T>(value));
    }

    std::string m_key;
    PlotlyMsg::DictItemVal m_item_val;
  };

  Dictionary() { reset(); }

  // template to create dictionary with arbitrary amount of item pair
  template <typename T, typename... Types>
  Dictionary(const std::string &key, T val, Types... rest)
      : Dictionary(rest...) {
    add_kwargs(key, val);
  }

  // template bases-case
  template <typename T>
  Dictionary(const std::string &key, T val) : Dictionary() {
    add_kwargs(key, val);
  }

  // template to create dictionary with dict item pair
  explicit Dictionary(DictionaryItemPair pair) : Dictionary() {
    add_kwargs(pair);
  }

  // template bases-case
  template <typename... Ts, typename = DictionaryItemPair>
  explicit Dictionary(DictionaryItemPair pair, Ts... rest)
      : Dictionary(rest...) {
    add_kwargs(pair);
  }

  // copy-construct
  Dictionary(const Dictionary &dict) {
    reset();
    m_msg->ParseFromString(dict.m_msg->SerializeAsString());
  }

  // swap-construct
  Dictionary(Dictionary &dict) {
    reset();
    m_msg.swap(dict.m_msg);
  }

  // rvalue-construct
  Dictionary(Dictionary &&dict) noexcept { m_msg = std::move(dict.m_msg); }

  //// ONLY ENABLE FOR CERTAIN CLASS
  //  template <bool...> struct bool_pack {};
  //  template <class... Ts>
  //  using conjunction = std::is_same<bool_pack<true, Ts::value...>,
  //                                   bool_pack<Ts::value..., true>>;
  //
  //  template <typename... Ts>
  //  using AllDictionaryItemPair = typename std::enable_if<
  //      conjunction<std::is_convertible<Ts,
  //      DictionaryItemPair>...>::value>::type;

  // methods to add kwargs into the dictionary

  template <typename T> void add_kwargs(const std::string &key, T value) {
    // pass the DictItemVal reference to helper function as template
    Plotly::_set_DictItemVal((*m_msg->mutable_data())[key],
                             std::forward<T>(value));
  }

  void add_kwargs(DictionaryItemPair &value) const;

  // directly set kwargs
  void set_kwargs(Plotly::Dictionary &value) const;

  void set_kwargs(Plotly::Dictionary &&value) const;

  std::unique_ptr<Dictionary> deep_copy() const;

  friend std::ostream &operator<<(std::ostream &out, Dictionary const &dict) {
    return out << (*dict.m_msg).data();
  }

  void reset();

  DictionaryMsg *release_ptr();

  // variables
  std::unique_ptr<DictionaryMsg> m_msg;
};

////////////////////////////////////////
// Trace class implementation
////////////////////////////////////////
class Trace {
public:
  Trace(PlotlyMsg::Trace::CreationMethods method, std::string method_func,
        Dictionary &kwargs)
      : m_method(method), m_method_func(std::move(method_func)) {
    m_kwargs.set_kwargs(kwargs);
  }
  Trace(PlotlyMsg::Trace::CreationMethods method, std::string method_func,
        Dictionary &&kwargs = {})
      : m_method(method), m_method_func(std::move(method_func)) {
    m_kwargs.set_kwargs(std::forward<Dictionary>(kwargs));
  }

  Trace() : Trace(PlotlyMsg::Trace::graph_objects, "") {}

  friend std::ostream &operator<<(std::ostream &out, Trace const &fig) {
    out << "trace<" << fig.m_method << "|" << fig.m_method_func << "|"
        << fig.m_kwargs << ">";
    return out;
  }

  PlotlyMsg::Trace::CreationMethods m_method;
  std::string m_method_func;
  Dictionary m_kwargs;
};

////////////////////////////////////////
// Figure class implementation
////////////////////////////////////////

class Figure {
public:
  explicit Figure(const std::string &uuid = "default") { reset(); }

  void set_uuid(const std::string &_uuid) { m_uuid = _uuid; }

  void set_trace_kwargs(int idx, Plotly::Dictionary &value) {
    if (idx >= size())
      throw std::out_of_range("Given index exceed the number of traces.");
    m_traces[idx].m_kwargs.set_kwargs(value);
  }

  void add_trace(Trace &trace) {
    // auto add trace if it's just one less than what we had
    add_trace();
    m_traces[size() - 1].m_kwargs.set_kwargs(trace.m_kwargs);
    m_traces[size() - 1].m_method = trace.m_method;
    m_traces[size() - 1].m_method_func = trace.m_method_func;
  }

  // r-value
  void add_trace(Trace &&trace) { add_trace(trace); }

  void add_trace(Dictionary &value) {
    // auto add trace if it's just one less than what we had
    add_trace();
    m_traces[size() - 1].m_kwargs.set_kwargs(value);
  }

  // r-value
  void add_trace(Dictionary &&value) {
    Dictionary new_dict(std::forward<Dictionary>(value));
    add_trace(new_dict);
  }

  template <typename T>
  void add_trace(const PlotlyMsg::Trace::CreationMethods method,
                 const std::string &method_func, T value) {
    add_trace(std::forward<T>(value));
    m_traces[size() - 1].m_method = method;
    m_traces[size() - 1].m_method_func = method_func;
  }

  template <typename T>
  void add_kwargs_to_trace(const std::string &key, T value) {
    // default index to the last trace
    add_kwargs_to_trace(size() - 1, key, std::forward<T>(value));
  }

  template <typename T>
  void add_kwargs_to_trace(int idx, const std::string &key, T value) {
    m_traces[idx].m_kwargs.add_kwargs(key, value);
  }

  int add_trace() {
    m_traces.emplace_back();
    return size();
  }

  Dictionary &get_trace(int idx) { return m_traces[idx].m_kwargs; }

  int size() const { return m_traces.size(); }

  Dictionary get_trace_copy(int idx) const {
    Dictionary new_copy(m_traces[idx].m_kwargs);
    return new_copy;
  }

  void send(zmq::send_flags send_flags = zmq::send_flags::dontwait);

  void reset();

  friend std::ostream &operator<<(std::ostream &out, Figure const &fig) {
    out << "Figure<" << fig.m_uuid << "|";
    for (auto &&trace : fig.m_traces) {
      out << trace;
    }
    out << ">";
    return out;
  }

  std::vector<Trace> m_traces;

private:
  zmq::message_t build_zmq_msg();

  // variables
  PlotlyMsg::MessageContainer m_msg;
  std::string m_uuid;
};

} // namespace Plotly
