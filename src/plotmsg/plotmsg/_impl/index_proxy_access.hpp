#pragma once

#include <chrono>
#include <iostream>
#include <thread>

#include <msg.pb.h>

#include <utility>
#include <zmq.hpp>


#include "helpers.hpp"

namespace PlotMsg {
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
} // namespace PlotMsg