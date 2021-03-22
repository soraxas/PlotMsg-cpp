#include "cpp2py_plotly.hpp"

namespace Plotly {

////////////////////////////////////////
// implementation of Dictionary
////////////////////////////////////////

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

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, bool value) {
  item_val.set_bool_(value);
}

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, double value) {
  item_val.set_double_(value);
}

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, int value) {
  item_val.set_int_(value);
}

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, const char *value) {
  item_val.set_string(value);
}

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val, std::string &value) {
  item_val.set_string(std::move(value));
}

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      const std::vector<double> &value) {
  item_val.set_allocated_series_d(vec_to_allocated_seriesD(value));
}

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      const std::vector<int> &value) {
  item_val.set_allocated_series_i(vec_to_allocated_seriesI(value));
}

void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      Plotly::Dictionary &value) {
  item_val.set_allocated_dict(value.release_ptr());
}

// r-value, uses l-value definition
void _set_DictItemVal(PlotlyMsg::DictItemVal &item_val,
                      Plotly::Dictionary &&value) {
  _set_DictItemVal(item_val, value);
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