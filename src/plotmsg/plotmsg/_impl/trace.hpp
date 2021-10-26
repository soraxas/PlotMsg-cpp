#pragma once

#include <chrono>
#include <iostream>
#include <thread>

#include <msg.pb.h>

#include <utility>
#include <zmq.hpp>


#include "helpers.hpp"

namespace PlotMsg {
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
} // namespace PlotMsg