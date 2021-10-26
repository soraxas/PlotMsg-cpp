#pragma once

#include <chrono>
#include <iostream>
#include <thread>

#include <msg.pb.h>

#include <utility>
#include <zmq.hpp>

#define PLOTMSG_DEFAULT_ADDR "tcp://127.0.0.1:5557"

namespace PlotMsg {

///////////////////////////////////////////////////////
// namespace ProtoMsg = PlotMsgProto;
using namespace PlotMsgProto;

///////////////////////////////////////////////////////

// define the static storage
inline std::unique_ptr<zmq::context_t> static_context;
inline std::unique_ptr<zmq::socket_t> static_publisher;

// static functions
void initialise_publisher(int sleep_after_bind = 1000,
                          const std::string &addr = PLOTMSG_DEFAULT_ADDR);

// easy alias
using DictionaryMsgData = google::protobuf::Map<std::string, DictItemValMsg>;

std::ostream &operator<<(std::ostream &out, DictionaryMsgData const &dict);

// forward declare
class Dictionary;

////////////////////////////////////////
// Helpers
////////////////////////////////////////

SeriesDMsg *vec_to_allocated_seriesD(std::vector<double> value);

SeriesIMsg *vec_to_allocated_seriesI(std::vector<int> value);

SeriesStringMsg *vec_to_allocated_seriesString(std::vector<std::string> value);

SeriesAnyMsg *vec_to_allocated_seriesAny(std::vector<SeriesAnyMsg_value> value);

// helper function to assign given DictItemValMsg with T value
void _set_DictItemVal(DictItemValMsg &item_val, bool value);

void _set_DictItemVal(DictItemValMsg &item_val, double value);

void _set_DictItemVal(DictItemValMsg &item_val, int value);

template <typename T> void _set_DictItemVal(DictItemValMsg &item_val, T value) {
  // this works for l/r-value std::string, and const char.
  item_val.set_string(std::forward<T>(value));
}

void _set_DictItemVal(DictItemValMsg &item_val,
                      const std::vector<double> &value);

void _set_DictItemVal(DictItemValMsg &item_val, const std::vector<int> &value);

void _set_DictItemVal(DictItemValMsg &item_val,
                      const std::vector<std::string> &value);

void _set_DictItemVal(DictItemValMsg &item_val,
                      const std::vector<SeriesAnyMsg_value> &value);

void _set_DictItemVal(DictItemValMsg &item_val, PlotMsg::Dictionary &value);

// r-value, uses l-value definition
void _set_DictItemVal(DictItemValMsg &item_val, PlotMsg::Dictionary &&value);

////////////////////////////////////////

struct IndexAccessProxy {
  /* This proxy is returned when accessing fig or trace. This proxy object
   * will in-turn forward the assignment when user assign value to this proxy.
   */
  PlotMsg::DictionaryMsgData &ref_data;
  std::string m_key;
  IndexAccessProxy(PlotMsg::DictionaryMsgData &ref_data, std::string key)
      : ref_data(ref_data), m_key(std::move(key)) {}

  template <typename T> IndexAccessProxy &operator=(T &other) {
    PlotMsg::_set_DictItemVal(ref_data[m_key], std::forward<T>(other));
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
      PlotMsg::_set_DictItemVal(m_item_val, std::forward<T>(value));
    }

    std::string m_key;
    DictItemValMsg m_item_val;
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
    // pass the DictItemValMsg reference to helper function as template
    PlotMsg::_set_DictItemVal((*m_msg->mutable_data())[key],
                              std::forward<T>(value));
  }

  void add_kwargs(DictionaryItemPair &value) const;

  // directly set kwargs
  void set_kwargs(PlotMsg::Dictionary &value) const;

  void set_kwargs(PlotMsg::Dictionary &&value) const;

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
  Trace() : Trace(PlotlyTrace::graph_objects, "") {}

  Trace(PlotlyTrace::CreationMethods method, std::string method_func,
        Dictionary &kwargs);

  Trace(PlotlyTrace::CreationMethods method, std::string method_func,
        Dictionary &&kwargs = {});

  IndexAccessProxy operator[](const std::string &key) const {
    return m_kwargs[key];
  }

  // assignment
  Trace &operator=(const Trace &&trace) noexcept {
    m_kwargs.set_kwargs(Dictionary(trace.m_kwargs));
    m_method = trace.m_method;
    m_method_func = trace.m_method_func;
    return *this;
  }

  // copy-construct
  Trace(const Trace &trace) {
    m_kwargs.set_kwargs(Dictionary(trace.m_kwargs));
    m_method = trace.m_method;
    m_method_func = trace.m_method_func;
  }

  // rvalue-construct
  Trace(Trace &&trace) noexcept {
    m_kwargs.set_kwargs(trace.m_kwargs);
    m_method = trace.m_method;
    m_method_func = trace.m_method_func;
  }

  friend std::ostream &operator<<(std::ostream &out, Trace const &fig);

  PlotlyTrace::CreationMethods m_method;
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

  void set_trace_kwargs(int idx, PlotMsg::Dictionary &value);

  void add_trace(Trace &trace) {
    _add_trace();
    m_traces[size() - 1].m_kwargs.set_kwargs(trace.m_kwargs);
    m_traces[size() - 1].m_method = trace.m_method;
    m_traces[size() - 1].m_method_func = trace.m_method_func;
  }

  // r-value
  void add_trace(Trace &&trace) { add_trace(trace); }

  // perfect forward all arguments to create trace
  template <typename... Ts> void add_trace(Ts... args) {
    add_trace(PlotMsg::Trace(std::forward<Ts>(args)...));
  }

  /*
  void add_trace(Dictionary &value);

  // r-value
  void add_trace(Dictionary &&value);
   */

  void remove_trace(uint index) {
    assert(index >= 0 && index < size());
    // naive way to erase, as the following creates segment fault
    //    m_traces.erase(m_traces.begin() + index);
    std::vector<Trace> tmp_traces;
    std::swap(tmp_traces, m_traces);
    m_traces.reserve(tmp_traces.size() - 1);
    for (uint i = 0; i < tmp_traces.size(); ++i) {
      if (i == index)
        continue;
      m_traces.push_back(tmp_traces[i]);
    }
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

  int _add_trace();

  void add_command(const std::string &func, Dictionary &value);

  void add_command(const std::string &func, Dictionary &&value);

  Dictionary &get_trace(int idx) {
    if (idx < 0)
      idx += size();
    return m_traces[idx].m_kwargs;
  }

  Trace &trace(int idx);

  int size() const { return m_traces.size(); }

  Trace get_trace_copy(int idx) const {
    Trace new_trace;
    Dictionary new_copy_kwargs(m_traces[idx].m_kwargs);
    new_trace.m_method_func = m_traces[idx].m_method_func;
    new_trace.m_method = m_traces[idx].m_method;
    new_trace.m_kwargs.set_kwargs(new_copy_kwargs);
    return new_trace;
  }

  Figure copy() const {
    Figure new_fig;
    for (int i = 0; i < size(); ++i)
      new_fig.add_trace(get_trace_copy(i));
    return new_fig;
  }

  void send(zmq::send_flags send_flags = zmq::send_flags::dontwait);

  void reset();

  friend std::ostream &operator<<(std::ostream &out, Figure const &fig);

  std::vector<Trace> m_traces;

private:
  zmq::message_t build_zmq_msg();

  // variables
  MessageContainer m_msg;
  std::string m_uuid;
};

////////////////////////////////////////
// Series Any (vector of {int,double,string,null})
////////////////////////////////////////

enum NullValueType { value };
NullValueType NullValue = NullValueType::value;

std::vector<SeriesAnyMsg_value> seriesAny_vector_create() {
  return std::vector<SeriesAnyMsg_value>();
}

void seriesAny_vector_push_back(std::vector<SeriesAnyMsg_value> &vec,
                                NullValueType null);

void seriesAny_vector_push_back(std::vector<SeriesAnyMsg_value> &vec,
                                const std::string &val);

void seriesAny_vector_push_back(std::vector<SeriesAnyMsg_value> &vec,
                                double val);

void seriesAny_vector_push_back(std::vector<SeriesAnyMsg_value> &vec, int val);

//////////
template <typename T, typename... Ts>
void seriesAny_vector(std::vector<SeriesAnyMsg_value> &vec, T first,
                      Ts... args) {
  // intermediate worker
  seriesAny_vector_push_back(vec, first);
  seriesAny_vector(vec, args...);
}

template <typename T>
void seriesAny_vector(std::vector<SeriesAnyMsg_value> &vec, T first) {
  // last worker
  seriesAny_vector_push_back(vec, first);
}

template <typename... Ts>
std::vector<SeriesAnyMsg_value> seriesAny_vector(Ts... args) {
  // entrypoint
  std::vector<SeriesAnyMsg_value> vec = seriesAny_vector_create();
  seriesAny_vector(vec, args...);
  return vec;
}
//////////

} // namespace PlotMsg
