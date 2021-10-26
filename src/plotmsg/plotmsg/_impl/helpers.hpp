#pragma once

#include <msg.pb.h>

namespace PlotMsg {

using namespace PlotMsgProto;

// forward declare
class Dictionary;

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

void _deep_copy_helper(const DictionaryMsgData &ori_dict,
                       DictionaryMsgData &new_dict);

} // namespace PlotMsg