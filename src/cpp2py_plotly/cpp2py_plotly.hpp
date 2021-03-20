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
// void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
//                        bool value);
//
// void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
//                        double value);
//
// void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
//                        int value);
//
// void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
//                        const std::string &value);
//
// void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
//                        const char *value);
//
// void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
//                        const std::vector<double> &value);
//
// void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
//                        const std::vector<int> &value);
//
// void dictmsg_add_kwargs(DictionaryMsgData *msg, const std::string &key,
//                        Plotly::Dictionary &value);

std::ostream &operator<<(std::ostream &out, DictionaryMsgData const &dict) {
  out << "Dict(";
  bool first_item = true;
  for (auto &&it = dict.begin(); it != dict.end(); ++it) {
    if (first_item)
      first_item = false;
    else
      out << ", ";

    out << it->first << ":";
    auto itemVal = it->second;

    switch (itemVal.value_case()) {
    case itemVal.kSeriesD:
      out << "seriesD<..>";
      break;
    case itemVal.kSeriesI:
      out << "seriesI<..>";
      break;
    case itemVal.kBool:
      out << itemVal.bool_();
      break;
    case itemVal.kDict:
      out << itemVal.dict().data();
      break;
    case itemVal.kDouble:
      out << itemVal.double_();
      break;
    case itemVal.kInt:
      out << itemVal.int_();
      break;
    case itemVal.kString:
      out << itemVal.string();
      break;
    case itemVal.VALUE_NOT_SET:
      break;
    default:
      throw std::runtime_error(
          "Unimplemented DictItemVal " +
          std::to_string(static_cast<int>(itemVal.value_case())));
    };
  }
  out << ")";
  return out;
}

// class that represents dict representation in protobuf
class Dictionary {

public:
  //  template <typename T, typename... Types>
  //  static Dictionary get_dict(const std::string &key, T val) {
  //    add_kwargs(key, val);
  //  }
  //
  //  template <typename T>
  //  static Dictionary get_dict(const std::string &key, T val) {
  //    Dictionary dict;
  //    dict.add_kwargs(key, val);
  //  }

  Dictionary() { reset(); }


//  template <typename Types>
//      void _constructor_helper(const std::string &key, T val) {
//      add_kwargs(key, val);
//    }










//  template <typename T, typename Args>
//  void aa(std::string key, T val, Args args...) {
////      add_kwargs(key, val);
//  }
//
//  template <typename T>
//  void aa(std::string key, T val) {
////      add_kwargs(key, val);
//  }
//
//
////  template <typename Types>
//  template <typename T, typename Args
//
////            ,
////            class = typename std::enable_if
////            <
////                !std::is_lvalue_reference<T>::value
////            >::type
//           >
//  Dictionary(T &&val, Args args...){
//      T lval(val);
//      aa(lval, args);
//  }
//
//




template< class T >
using decay_t = typename std::decay<T>::type;

//
//  //  template <typename T>
//  template <typename T, typename Types>
//  Dictionary(const std::string &key, T val ,Types rest...)
//      : Dictionary(rest) {
//    add_kwargs(key, val);
//  }
//  template <typename T>
//  Dictionary(const std::string &key, T val)
//      : Dictionary() {
//    add_kwargs(key, val);
//  }


  //  template <typename T>
  template <typename T, typename... Types>
  Dictionary(const std::string &key, T val , Types ...rest)
      : Dictionary(rest...) {
    add_kwargs(key, val);
  }

  template <typename T>
  Dictionary(const std::string &key, T val)
      : Dictionary() {
    add_kwargs(key, val);
  }
















////  template <typename Types>
//  template <typename T, typename Args,
//            class = typename std::enable_if
//            <
//                std::is_lvalue_reference<T>::value
//            >::type
//           >
//  Dictionary(T val, Args args...){
////      aa(val, args);
//  }






  // copy-construct
  Dictionary(Dictionary &dict) {
    reset();
    m_msg.swap(dict.m_msg);
  }

  // rvalue-construct
  Dictionary(Dictionary &&dict) {
    m_msg = std::move(dict.m_msg);
  }

  /*



  //
  //  Dictionary(const Dictionary &dict) {
  //    reset();
  //    m_msg.swap(dict.m_msg);
  //  }

  //  Dictionary(Plotly::Dictionary &dict) {
  //    throw std::runtime_error("no");
  //  }

  //  Dictionary(Plotly::Dictionary &dict) {
  //    m_msg.swap(dict.m_msg);
  //  }

  //  Dictionary(const Plotly::Dictionary dict) {
  //    m_msg = std::unique_ptr<DictionaryMsg>(dict.m_msg);
  //  }

  template <typename T>
  Dictionary(const std::string &key, T val) : Dictionary() {
    _constructor_helper(key, val);
  }

  //  // rvalue as argument
  //  Dictionary(const std::string &key, Dictionary &&val) : Dictionary() {
  //    Dictionary lval(val);
  //    add_kwargs(key, lval);
  //  }

  // return invert_x< std::remove_reference_t<P> >( pin,
  //                                                std::is_lvalue_reference<P&&>());
  // return invert_x< std::remove_reference_t<P> >( pin,
  //                                                std::is_lvalue_reference<P&&>());

  // template<typename T, typename Types,
  //            std::enable_if_t<!std::is_rvalue_reference<T
  //     >::value, int> = 0>

  //  template <typename T,
  //  std::enable_if_t<!std::is_rvalue_reference<Dictionary
  //     >::value, int> = 0>

  //  template <typename T,
  //  std::enable_if_t<!std::is_rvalue_reference<Dictionary
  //     >::value, int> = 0>
  //  void _constructor_helper(const std::string &key, T val) {
  //    add_kwargs(key, val);
  //  }

  void _constructor_helper(const std::string &key, Dictionary &val) {
    Dictionary lval(val);
    add_kwargs(key, lval);
  }

//  template <typename T>
  template <typename T, typename Types,
            class = typename std::enable_if
            <
                !std::is_lvalue_reference<T>::value
            >::type
           >
  Dictionary(
      const std::string &key, T &&val
//      const std::string &key1, T &&val1
             ,Types rest...
             )
      : Dictionary() {

    //    Dictionary lval(val);
    add_kwargs(key, val);

    //    Dictionary lval(val);
    _constructor_helper(key, val);
    //    _constructor_helper(key, Dictionary());
    //    _constructor_helper(key, lval);
  }





  */









  //  template <typename T, typename... Types>
  //  Dictionary(const std::string &key, Dictionary && val, Types... rest)
  //      : Dictionary(rest...) {
  //    _constructor_helper(key, val);
  //
  //  }

  //
  //  // rvalue as argument
  //  template <typename... Types>
  //  Dictionary(const std::string &key, Dictionary &&val, Types... rest)
  //      : Dictionary(rest...) {
  //    Dictionary lval(val);
  //    add_kwargs(key, lval);
  //  }

  //  template <typename T, typename... Types>
  //  Dictionary(const std::string &key, T val, Types... rest)
  //      : Dictionary(rest...) {
  //    add_kwargs(key, val);
  //  }

  //
  //  template <typename T, typename... Types>
  //  Dictionary(const std::string &key, Dictionary &&val, Types... rest)
  //      : Dictionary(rest...) {
  //    add_kwargs(key, val);
  //  }

  //  template <typename T, typename... Types>
  //  Dictionary(std::string key, Dictionary val, Types... rest) :
  //  Dictionary(rest...) {
  //    add_kwargs(key, val);
  //  }

  void add_kwargs(const std::string &key, bool value) const;

  void add_kwargs(const std::string &key, double value) const;

  void add_kwargs(const std::string &key, int value) const;

  void add_kwargs(const std::string &key, const std::string &value) const;

  void add_kwargs(const std::string &key, const char *value) const;

  void add_kwargs(const std::string &key,
                  const std::vector<double> &value) const;

  void add_kwargs(const std::string &key, const std::vector<int> &value) const;

  void add_kwargs(const std::string &key, Plotly::Dictionary &value) const;

  void set_kwargs(Plotly::Dictionary &value) const;

  friend std::ostream &operator<<(std::ostream &out, Dictionary const &dict) {
    return out << (*dict.m_msg).data();
  }
  void reset();

  DictionaryMsg *release_ptr();

  // variables
  std::unique_ptr<DictionaryMsg> m_msg;
};

////////////////////////////////////////
// Dictionary Builder
////////////////////////////////////////

template <typename T>
static void _get_dict_helper(const Dictionary &dict, const std::string &key,
                             T val) {
  dict.add_kwargs(key, val);
}

template <typename T, typename... Types>
static void _get_dict_helper(const Dictionary &dict, const std::string &key,
                             T val, Types... rest) {
  dict.add_kwargs(key, val);
  _get_dict_helper(dict, rest...);
}

template <typename T, typename... Types>
static void _get_dict_helper(const Dictionary &dict, const std::string &key,
                             Dictionary val, Types... rest) {
  dict.add_kwargs(key, val);
  _get_dict_helper(dict, rest...);
}

template <typename... Args> static Dictionary get_dict(Args... args) {
  Dictionary dict;
  _get_dict_helper(dict, args...);
  return dict;
}

////////////////////////////////////////

class Figure {
public:
  explicit Figure(std::string uuid = "default");

  void set_uuid(const std::string &_uuid) {
    m_uuid = _uuid;
    m_msg.set_uuid(m_uuid);
  }

  //  template <typename T>
  //  void add_kwargs(const std::string &key, const T &value) {
  //    dictmsg_add_kwargs(msg.mutable_kwargs()->mutable_data(), key, value);
  //  }

  template <typename T> void add_kwargs(const std::string &key, T &value) {
    //    dictmsg_add_kwargs(msg.mutable_kwargs()->mutable_data(), key, value);
    m_kwargs.add_kwargs(key, value);
  }

  void set_kwargs(Dictionary &value) {
    m_kwargs.set_kwargs(value);
  }

  // by r-value
  void set_kwargs(Dictionary &&value) {
    Dictionary lval(value);
    m_kwargs.set_kwargs(lval);
  }

  const Dictionary get_kwargs() {
    return m_kwargs;
  }

  void send(zmq::send_flags send_flags = zmq::send_flags::dontwait);

  void reset();

private:
  zmq::message_t build_zmq_msg();

  // variables
  PlotlyMsg::Figure m_msg;
  std::string m_uuid;
  Dictionary m_kwargs;
};

////////////////////////////////////////
// Helpers
////////////////////////////////////////
PlotlyMsg::SeriesD *vec_to_allocated_seriesD(std::vector<double> value);

PlotlyMsg::SeriesI *vec_to_allocated_seriesI(std::vector<int> value);

} // namespace Plotly
