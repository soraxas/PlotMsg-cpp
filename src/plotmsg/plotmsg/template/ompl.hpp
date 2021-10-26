#pragma once

#include "plotmsg/main.hpp"
#include <ompl/base/PlannerData.h>

namespace Plotly {
namespace OmplTemplate {

namespace ob = ompl::base;
namespace og = ompl::geometric;

void plot_planner_data_graph(Plotly::Figure &fig, const ob::PlannerData &data) {
  // plot all edges
  std::vector<std::pair<double, double>> xs_e;
  std::vector<std::pair<double, double>> ys_e;
  std::vector<unsigned int> edgesList;
  for (uint i = 0; i < data.numVertices(); ++i) {
    const auto &v1 = data.getVertex(i);
    auto v1_pos = v1.getState()->as<StateType>()->values;
    data.getEdges(i, edgesList);
    for (auto &&j : edgesList) {
      const auto &v2 = data.getVertex(j);
      auto v2_pos = v2.getState()->as<StateType>()->values;
      xs_e.emplace_back(v1_pos[0], v2_pos[0]);
      ys_e.emplace_back(v1_pos[1], v2_pos[1]);
    }
  }
  fig.add_trace(Plotly::TraceTemplate::edges(xs_e, ys_e));
  fig.get_trace(-1)["name"] = "graph";

  // plot start/target
  std::vector<double> xs;
  std::vector<double> ys;
  std::vector<std::string> cs;
  for (uint i = 0; i < data.numStartVertices(); ++i) {
    auto v_pos = data.getStartVertex(i).getState()->as<StateType>()->values;
    xs.push_back(v_pos[0]);
    ys.push_back(v_pos[1]);
    cs.emplace_back("green");
  }
  for (uint i = 0; i < data.numGoalVertices(); ++i) {
    auto v_pos = data.getGoalVertex(i).getState()->as<StateType>()->values;
    xs.push_back(v_pos[0]);
    ys.push_back(v_pos[1]);
    cs.emplace_back("red");
  }
  fig.add_trace(Plotly::TraceTemplate::vertices(xs, ys, cs));
  fig.get_trace(-1)["showlegend"] = false;
}

/*
 * Assume the given path have a 2-dimensional state
 * */
template <typename StateType>
void plot_path(Plotly::Figure &fig, const og::PathGeometric &path,
               std::string colour = "blue", std::string name = "solution") {
  assert(path.getState(0)->as<StateType>()->getDimension() == 2);
  std::vector<double> xs, ys;
  for (uint i = 0; i < path.getStateCount(); ++i) {
    auto pos = path.getState(i)->as<StateType>()->values;
    xs.push_back(pos[0]);
    ys.push_back(pos[1]);
  }
  auto trace = Plotly::TraceTemplate::scatter(xs, ys);
  trace["name"] = name;
  trace["mode"] = "lines";
  trace["line_width"] = 1.5;
  trace["line_color"] = colour;

  fig.add_trace(trace);
}

} // namespace OmplTemplate
} // namespace Plotly