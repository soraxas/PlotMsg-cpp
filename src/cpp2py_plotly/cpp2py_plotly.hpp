#pragma once

#include <chrono>
#include <iostream>
#include <thread>

#include <msg.pb.h>

#include <utility>
#include <zmq.hpp>

#define CPP2PY_PLOTLY_DEFAULT_ADDR "tcp://127.0.0.1:5557"

namespace Plotly {
// define the static storage
std::unique_ptr<zmq::context_t> static_context;
std::unique_ptr<zmq::socket_t> static_publisher;

// static functions
void initialise_publisher(int sleep_after_bind = 1000,
                          const std::string &addr = CPP2PY_PLOTLY_DEFAULT_ADDR);

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

PlotlyMsg::SeriesString *
vec_to_allocated_seriesString(std::vector<std::string> value);

PlotlyMsg::SeriesAny *
vec_to_allocated_seriesAny(std::vector<PlotlyMsg::SeriesAny_value> value);

// helper function to assign given PlotlyMsg::DictItemVal with T value
void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, bool value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, double value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, int value);

template <typename T>
void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, T value) {
  // this works for l/r-value std::string, and const char.
  item_val.set_string(std::forward<T>(value));
}

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      const std::vector<double> &value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      const std::vector<int> &value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      const std::vector<std::string> &value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      const std::vector<PlotlyMsg::SeriesAny_value> &value);

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      Plotly::Dictionary &value);

// r-value, uses l-value definition
void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      Plotly::Dictionary &&value);

////////////////////////////////////////

struct IndexAccessProxy {
  /* This proxy is returned when accessing fig or trace. This proxy object
   * will in-turn forward the assignment when user assign value to this proxy.
   */
  Plotly::DictionaryMsgData &ref_data;
  std::string m_key;
  IndexAccessProxy(Plotly::DictionaryMsgData &ref_data, std::string key)
      : ref_data(ref_data), m_key(std::move(key)) {}

  template <typename T> IndexAccessProxy &operator=(T &other) {
    Plotly::_set_DictItemVal(ref_data[m_key], std::forward<T>(other));
    return *this;
  }

  // r-value
  template <typename T> IndexAccessProxy &operator=(T &&other) {
    return operator=(other);
  }

  IndexAccessProxy operator[](const std::string &key) const {
    return IndexAccessProxy(*ref_data[m_key].mutable_dict()->mutable_data(),
                            key);
  }
};

////////////////////////////////////////

// class that represents dict representation in protobuf
class Dictionary {
public:
  struct DictionaryItemPair {
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

  IndexAccessProxy operator[](const std::string &key) const {
    return IndexAccessProxy(*m_msg->mutable_data(), key);
  }

  ///////////////////////////////////////////////////

  friend std::ostream &operator<<(std::ostream &out, Dictionary const &dict);

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
  Trace() : Trace(PlotlyMsg::Trace::graph_objects, "") {}

  Trace(PlotlyMsg::Trace::CreationMethods method, std::string method_func,
        Dictionary &kwargs);

  Trace(PlotlyMsg::Trace::CreationMethods method, std::string method_func,
        Dictionary &&kwargs = {});

  IndexAccessProxy operator[](const std::string &key) const {
    return m_kwargs[key];
  }

  friend std::ostream &operator<<(std::ostream &out, Trace const &fig);

  PlotlyMsg::Trace::CreationMethods m_method;
  std::string m_method_func;
  Dictionary m_kwargs;
};

////////////////////////////////////////
// Figure class implementation
////////////////////////////////////////

class Figure {
public:
  explicit Figure(std::string uuid = "default") : m_uuid(std::move(uuid)) {
    reset();
  }

  void set_uuid(const std::string &_uuid) { m_uuid = _uuid; }

  void set_trace_kwargs(int idx, Plotly::Dictionary &value);

  void add_trace(Trace &trace) {
    // auto add trace if it's just one less than what we had
    _add_trace();
    m_traces[size() - 1].m_kwargs.set_kwargs(trace.m_kwargs);
    m_traces[size() - 1].m_method = trace.m_method;
    m_traces[size() - 1].m_method_func = trace.m_method_func;
  }

  // r-value
  void add_trace(Trace &&trace) { add_trace(trace); }

  // perfect forward all arguments to create trace
  template <typename... Ts> void add_trace(Ts... args) {
    add_trace(Plotly::Trace(std::forward<Ts>(args)...));
  }

  /*
  void add_trace(Dictionary &value);

  // r-value
  void add_trace(Dictionary &&value);
   */

  template <typename T>
  void add_kwargs_to_trace(const std::string &key, T value) {
    // default index to the last trace
    add_kwargs_to_trace(size() - 1, key, std::forward<T>(value));
  }

  template <typename T>
  void add_kwargs_to_trace(int idx, const std::string &key, T value) {
    m_traces[idx].m_kwargs.add_kwargs(key, value);
  }

  int _add_trace();

  void add_command(const std::string &func, Dictionary &value);

  void add_command(const std::string &func, Dictionary &&value);

  Dictionary &get_trace(int idx) { return m_traces[idx].m_kwargs; }

  Trace &trace(int idx);

  int size() const { return m_traces.size(); }

  Dictionary get_trace_copy(int idx) const {
    Dictionary new_copy(m_traces[idx].m_kwargs);
    return new_copy;
  }

  void send(zmq::send_flags send_flags = zmq::send_flags::dontwait);

  void reset();

  friend std::ostream &operator<<(std::ostream &out, Figure const &fig);

  std::vector<Trace> m_traces;

private:
  zmq::message_t build_zmq_msg();

  // variables
  PlotlyMsg::MessageContainer m_msg;
  std::string m_uuid;
};

////////////////////////////////////////
// Series Any (vector of {int,double,string,null})
////////////////////////////////////////

enum NullValueType { value };
NullValueType NullValue = NullValueType::value;

std::vector<PlotlyMsg::SeriesAny_value> seriesAny_vector_create() {
  return std::vector<PlotlyMsg::SeriesAny_value>();
}

void seriesAny_vector_push_back(std::vector<PlotlyMsg::SeriesAny_value> &vec,
                                NullValueType null);

void seriesAny_vector_push_back(std::vector<PlotlyMsg::SeriesAny_value> &vec,
                                const std::string &val);

void seriesAny_vector_push_back(std::vector<PlotlyMsg::SeriesAny_value> &vec,
                                double val);

void seriesAny_vector_push_back(std::vector<PlotlyMsg::SeriesAny_value> &vec,
                                int val);

//////////
template <typename T, typename... Ts>
void seriesAny_vector(std::vector<PlotlyMsg::SeriesAny_value> &vec, T first,
                      Ts... args) {
  // intermediate worker
  seriesAny_vector_push_back(vec, first);
  seriesAny_vector(vec, args...);
}

template <typename T>
void seriesAny_vector(std::vector<PlotlyMsg::SeriesAny_value> &vec, T first) {
  // last worker
  seriesAny_vector_push_back(vec, first);
}

template <typename... Ts>
std::vector<PlotlyMsg::SeriesAny_value> seriesAny_vector(Ts... args) {
  // entrypoint
  std::vector<PlotlyMsg::SeriesAny_value> vec = seriesAny_vector_create();
  seriesAny_vector(vec, args...);
  return vec;
}
//////////

} // namespace Plotly
