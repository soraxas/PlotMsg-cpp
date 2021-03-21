#include "cpp2py_plotly.hpp"

namespace Plotly {

////////////////////////////////////////
// implementation of Dictionary
////////////////////////////////////////

void Dictionary::add_kwargs(const std::string &key, bool value) const {
  (*m_msg->mutable_data())[key].set_bool_(value);
}

void Dictionary::add_kwargs(const std::string &key, double value) const {
  (*m_msg->mutable_data())[key].set_double_(value);
}

void Dictionary::add_kwargs(const std::string &key, int value) const {
  (*m_msg->mutable_data())[key].set_int_(value);
}

void Dictionary::add_kwargs(const std::string &key, const char *value) const {
  (*m_msg->mutable_data())[key].set_string(value);
}

void Dictionary::add_kwargs(const std::string &key,
                            const std::string &value) const {
  (*m_msg->mutable_data())[key].set_string(value);
}

void Dictionary::add_kwargs(const std::string &key,
                            const std::vector<double> &value) const {
  (*m_msg->mutable_data())[key].set_allocated_series_d(
      vec_to_allocated_seriesD(value));
}

void Dictionary::add_kwargs(const std::string &key,
                            const std::vector<int> &value) const {
  (*m_msg->mutable_data())[key].set_allocated_series_i(
      vec_to_allocated_seriesI(value));
}

void Dictionary::add_kwargs(const std::string &key,
                            Plotly::Dictionary &value) const {
  (*m_msg->mutable_data())[key].set_allocated_dict(value.release_ptr());
}

void Dictionary::add_kwargs(const std::string &key,
                            PlotlyMsg::DictItemVal &value) const {
  (*m_msg->mutable_data())[key].Swap(&value);
}

void Dictionary::add_kwargs(
    Plotly::Dictionary::DictionaryItemPair &value) const {
  (*m_msg->mutable_data())[value.m_key].Swap(&value.m_item_val);
}

void Dictionary::set_kwargs(Plotly::Dictionary &value) const {
  m_msg->mutable_data()->swap(*value.m_msg->mutable_data());
}

void Dictionary::reset() { m_msg = std::make_unique<DictionaryMsg>(); }

DictionaryMsg *Dictionary::release_ptr() { return m_msg.release(); }

////////////////////////////////////////
// implementation of Dictionary Item Pair
////////////////////////////////////////

Dictionary::DictionaryItemPair::DictionaryItemPair(const std::string &key,
                                                   bool value) {
  m_key = key;
  m_item_val.set_bool_(value);
}

Dictionary::DictionaryItemPair::DictionaryItemPair(const std::string &key,
                                                   double value) {
  m_key = key;
  m_item_val.set_double_(value);
}

Dictionary::DictionaryItemPair::DictionaryItemPair(const std::string &key,
                                                   int value) {
  m_key = key;
  m_item_val.set_int_(value);
}

Dictionary::DictionaryItemPair::DictionaryItemPair(const std::string &key,
                                                   const char *value) {
  m_key = key;
  m_item_val.set_string(value);
}

Dictionary::DictionaryItemPair::DictionaryItemPair(const std::string &key,
                                                   std::string &value) {
  m_key = key;
  m_item_val.set_string(std::move(value));
}

Dictionary::DictionaryItemPair::DictionaryItemPair(
    const std::string &key, const std::vector<double> &value) {
  m_key = key;
  m_item_val.set_allocated_series_d(vec_to_allocated_seriesD(value));
}

Dictionary::DictionaryItemPair::DictionaryItemPair(
    const std::string &key, const std::vector<int> &value) {
  m_key = key;
  m_item_val.set_allocated_series_i(vec_to_allocated_seriesI(value));
}

Dictionary::DictionaryItemPair::DictionaryItemPair(const std::string &key,
                                                   Plotly::Dictionary &value) {
  m_key = key;
  m_item_val.set_allocated_dict(value.release_ptr());
}

Dictionary::DictionaryItemPair::DictionaryItemPair(const std::string &key,
                                                   Plotly::Dictionary &&value) {
  Plotly::Dictionary lvalue(value);
  m_key = key;
  m_item_val.set_allocated_dict(lvalue.release_ptr());
}

////////////////////////////////////////
// implementation of Plotly Figure
////////////////////////////////////////
Figure::Figure(std::string uuid) : m_uuid(std::move(uuid)) { reset(); }

void Figure::send(zmq::send_flags send_flags) {
  initialise_publisher();
  // swap kwargs in the dictionary container with the protobuf internal msg
  auto kwargs_ptr = m_kwargs.release_ptr();
  m_msg.mutable_kwargs()->Swap(kwargs_ptr);
  m_kwargs.m_msg = std::unique_ptr<DictionaryMsg>(kwargs_ptr);

  static_publisher->send(build_zmq_msg(), send_flags);
}

void Figure::reset() {
  m_msg.Clear();
  m_msg.set_uuid(m_uuid);
}
zmq::message_t Figure::build_zmq_msg() {
  // serial protobuf to string
  std::string encoded_msg;
  m_msg.SerializeToString(&encoded_msg);
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

//////////////////////////////////////////////////////////////////////////
}; // namespace Plotly