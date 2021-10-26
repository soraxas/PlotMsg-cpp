#pragma once

#include <chrono>
#include <iostream>
#include <thread>

#include <msg.pb.h>

#include <utility>
#include <zmq.hpp>

#include "trace.hpp"
#include "helpers.hpp"

namespace PlotMsg {

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
} // namespace PlotMsg
