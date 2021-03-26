#include "cpp2py_plotly.hpp"

namespace Plotly {

////////////////////////////////////////
// implementation of Dictionary
////////////////////////////////////////

void Dictionary::add_kwargs(
    Plotly::Dictionary::DictionaryItemPair &value) const {
  (*m_msg->mutable_data())[value.m_key].Swap(&value.m_item_val);
}

void Dictionary::set_kwargs(Plotly::Dictionary &&value) const {
  Dictionary lvalue(value);
  set_kwargs(lvalue);
}

void Dictionary::set_kwargs(Plotly::Dictionary &value) const {
  m_msg->mutable_data()->swap(*value.m_msg->mutable_data());
}

void _deep_copy_helper(const DictionaryMsgData &ori_dict,
                       DictionaryMsgData &new_dict) {
  /*
   * Copy contents from ori_dict to new_dict
   */
  for (auto &&it = ori_dict.begin(); it != ori_dict.end(); ++it) {
    auto key = it->first;
    auto itemVal = it->second;

    if (itemVal.value_case() == PlotlyMsg::DictItemVal::kSeriesD) {
      // TODO
      auto *series_d = new PlotlyMsg::SeriesD();
      series_d->ParseFromString(itemVal.series_d().SerializeAsString());
      new_dict[key].set_allocated_series_d(series_d);
    } else if (itemVal.value_case() == PlotlyMsg::DictItemVal::kSeriesI) {
      // TODO
      auto *series_i = new PlotlyMsg::SeriesI();
      series_i->ParseFromString(itemVal.series_i().SerializeAsString());
      new_dict[key].set_allocated_series_i(series_i);
      itemVal.series_i().New(itemVal.GetArena());
    } else if (itemVal.value_case() == PlotlyMsg::DictItemVal::kBool) {
      _set_DictItemVal(new_dict[key], itemVal.bool_());
    } else if (itemVal.value_case() == PlotlyMsg::DictItemVal::kDict) {
      auto *nested_dict = new DictionaryMsg();
      _deep_copy_helper(itemVal.dict().data(),
                        (*(*nested_dict).mutable_data()));
      new_dict[key].set_allocated_dict(nested_dict);
    } else if (itemVal.value_case() == PlotlyMsg::DictItemVal::kDouble) {
      _set_DictItemVal(new_dict[key], itemVal.double_());
    } else if (itemVal.value_case() == PlotlyMsg::DictItemVal::kString) {
      std::string new_str = itemVal.string();
      _set_DictItemVal(new_dict[key], new_str);
    } else {
      throw std::runtime_error(
          "Unimplemented DictItemVal " +
          std::to_string(static_cast<int>(itemVal.value_case())));
    };
  }
}

std::unique_ptr<Dictionary> Dictionary::deep_copy() const {
  Dictionary dict;
  _deep_copy_helper(m_msg->data(), *dict.m_msg->mutable_data());
  return std::make_unique<Dictionary>(dict);
}

void Dictionary::reset() { m_msg = std::make_unique<DictionaryMsg>(); }

DictionaryMsg *Dictionary::release_ptr() { return m_msg.release(); }

std::ostream &operator<<(std::ostream &out, const Dictionary &dict) {
  return out << (*dict.m_msg).data();
}

////////////////////////////////////////
// implementation of Plotly Figure
////////////////////////////////////////

void Figure::send(zmq::send_flags send_flags) {
  initialise_publisher();
  // swap kwargs in the dictionary container with the protobuf internal msg
  auto _fig = m_msg.mutable_fig();
  _fig->set_uuid(m_uuid);

  for (int i = 0; i < size(); ++i) {
    auto trace = _fig->add_traces();
    trace->mutable_kwargs()->Swap(m_traces[i].m_kwargs.release_ptr());
    trace->set_method(m_traces[i].m_method);
    trace->set_method_func(m_traces[i].m_method_func);
  }

  std::string encoded_msg;
  m_msg.SerializeToString(&encoded_msg);
  zmq::message_t zmq_msg(encoded_msg.size());
  memcpy((void *)zmq_msg.data(), encoded_msg.c_str(), encoded_msg.size());
  static_publisher->send(zmq_msg, send_flags);

  m_traces.clear();
}

void Figure::reset() {
  m_msg.Clear();
  m_traces.clear();
  m_msg.set_allocated_fig(new PlotlyMsg::Figure());
  set_uuid(m_uuid);
}
zmq::message_t Figure::build_zmq_msg() {
  // serial protobuf to string
  std::string encoded_msg;

  std::cout << "|>"
            << m_msg.mutable_fig()->mutable_traces()->Get(0).kwargs().data()
            << std::endl;
  m_msg.SerializeToString(&encoded_msg);
  // memcpy to a zmq m_msg
  std::cout << "size   " << encoded_msg.size() << std::endl;
  zmq::message_t zmq_msg(encoded_msg.size());
  memcpy((void *)zmq_msg.data(), encoded_msg.c_str(), encoded_msg.size());
  return zmq_msg;
}
std::ostream &operator<<(std::ostream &out, const Figure &fig) {
  out << "Figure<" << fig.m_uuid << "|";
  for (auto &&trace : fig.m_traces) {
    out << trace;
  }
  out << ">";
  return out;
}

void Figure::set_trace_kwargs(int idx, Dictionary &value) {
  if (idx >= size())
    throw std::out_of_range("Given index exceed the number of traces.");
  m_traces[idx].m_kwargs.set_kwargs(value);
}

void Figure::add_trace(Dictionary &value) {
  // auto add trace if it's just one less than what we had
  _add_trace();
  m_traces[size() - 1].m_kwargs.set_kwargs(value);
}
void Figure::add_trace(Dictionary &&value) {
  Dictionary new_dict(std::forward<Dictionary>(value));
  add_trace(new_dict);
}

int Figure::_add_trace() {
  m_traces.emplace_back();
  return size();
}

Trace &Figure::trace(int idx) {
  // return a reference to a trace
  if (idx < 0) {
    // negative indexing, wraps around.
    idx = size() + idx;
    if (idx < 0)
      throw std::out_of_range(
          "Given negative index exceed the number of traces.");
  }
  if (idx >= size()) {
    throw std::out_of_range("Given index exceed the number of traces.");
  }
  return m_traces[idx];
}

void Figure::add_command(const std::string &func, Dictionary &value) {
  auto cmd = m_msg.mutable_fig()->add_commands();
  cmd->set_func(func);
  cmd->mutable_kwargs()->Swap(value.release_ptr());
}

void Figure::add_command(const std::string &func, Dictionary &&value) {
  Dictionary lvalue(value);
  add_command(func, lvalue);
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

std::ostream &operator<<(std::ostream &out, const DictionaryMsgData &dict) {
  return out;
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
    case PlotlyMsg::DictItemVal::kSeriesD:
      out << "seriesD<..>";
      break;
    case PlotlyMsg::DictItemVal::kSeriesI:
      out << "seriesI<..>";
      break;
    case PlotlyMsg::DictItemVal::kBool:
      out << itemVal.bool_();
      break;
    case PlotlyMsg::DictItemVal::kDict:
      out << itemVal.dict().data();
      break;
    case PlotlyMsg::DictItemVal::kDouble:
      out << itemVal.double_();
      break;
    case PlotlyMsg::DictItemVal::kInt:
      out << itemVal.int_();
      break;
    case PlotlyMsg::DictItemVal::kString:
      out << itemVal.string();
      break;
    case PlotlyMsg::DictItemVal::VALUE_NOT_SET:
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
Trace::Trace(PlotlyMsg::Trace::CreationMethods method, std::string method_func,
             Dictionary &kwargs)
    : m_method(method), m_method_func(std::move(method_func)) {
  m_kwargs.set_kwargs(kwargs);
}
Trace::Trace(PlotlyMsg::Trace::CreationMethods method, std::string method_func,
             Dictionary &&kwargs)
    : m_method(method), m_method_func(std::move(method_func)) {
  m_kwargs.set_kwargs(std::forward<Dictionary>(kwargs));
}
std::ostream &operator<<(std::ostream &out, const Trace &fig) {
  out << "trace<" << fig.m_method << "|" << fig.m_method_func << "|"
      << fig.m_kwargs << ">";
  return out;
}
}; // namespace Plotly