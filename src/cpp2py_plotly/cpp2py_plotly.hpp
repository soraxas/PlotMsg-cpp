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
// Figure classplementation
////////////////////////////////////////

class Figure {
public:
  explicit Figure(std::string uuid = "default");

  void set_uuid(const std::string &_uuid) {
    m_uuid = _uuid;
    m_msg.set_uuid(m_uuid);
  }

  template <typename T> void add_kwargs(const std::string &key, T value) {
    m_kwargs.add_kwargs(key, std::forward<T>(value));
  }

  void set_kwargs(Dictionary &value) { m_kwargs.set_kwargs(value); }

  // by r-value (uses l-value definition)
  void set_kwargs(Dictionary &&value) { set_kwargs(value); }

  Dictionary get_dict_copy() const {
    Dictionary new_dict(m_kwargs);
    return new_dict;
  }

  void send(zmq::send_flags send_flags = zmq::send_flags::dontwait);

  Dictionary *mutable_kwargs();

  void reset();

  friend std::ostream &operator<<(std::ostream &out, Figure const &fig) {
    return out << "Figure<uuid:" << fig.m_uuid
               << "|kwargs=" << fig.m_kwargs << ">";
  }

private:
  zmq::message_t build_zmq_msg();

  // variables
  PlotlyMsg::Figure m_msg;
  std::string m_uuid;
  Dictionary m_kwargs;
};

} // namespace Plotly
