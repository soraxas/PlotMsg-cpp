#pragma once

#include "cpp2py_plotly.hpp"

/*
 * Implements tempalte message for easy usage
 */

namespace Plotly {
namespace TraceTemplate {
using ditem = Plotly::Dictionary::DictionaryItemPair;

Plotly::Trace Scatter() {
  return Plotly::Trace( //
      PlotlyMsg::Trace::graph_objects, "Scatter",
      Plotly::Dictionary(                     //
          "mode", "markers",                  //
          "marker_size", 10,                  //
          "marker_opacity", 0.9,              //
          "marker_colorscale", "Viridis",     //
          "marker_colorbar_title", "Colorbar" //
          )                                   //
  );
}

template <typename T1, typename T2>
Plotly::Trace Scatter(std::vector<T1> &x, std::vector<T2> &y) {
  Plotly::Trace trace = Scatter();
  trace.m_kwargs.add_kwargs("x", x);
  trace.m_kwargs.add_kwargs("y", y);
  return trace;
}

template <typename T1, typename T2>
Plotly::Trace ScatterWithColour(std::vector<T1> &x, std::vector<T1> &y,
                                std::vector<T2> &c) {
  Plotly::Trace trace = Scatter(x, y);
  trace.m_kwargs.add_kwargs("color", c);
  return trace;
}

template <typename T>
Plotly::Trace vector_field(std::vector<T> &x, std::vector<T> &y,
                           std::vector<T> &u, std::vector<T> &v) {
  return Plotly::Trace(PlotlyMsg::Trace::figure_factory, "create_quiver",
                       Plotly::Dictionary(    //
                           "x", x,            //
                           "y", y,            //
                           "u", u,            //
                           "v", v,            //
                           "scale", .25,      //
                           "arrow_scale", .4, //
                           "name", "quiver",  //
                           "line_width", 1    //
                           ));
}
// alias of vector field
template <typename... T> Plotly::Trace quiver(T... args) {
  return vector_field(args...);
}

template <typename T>
Plotly::Trace contour(std::vector<T> &x, std::vector<T> &y, std::vector<T> &c) {
  return Plotly::Trace(PlotlyMsg::Trace::graph_objects, "Contour",
                       Plotly::Dictionary( //
                           "x", x,         //
                           "y", y,         //
                           "z", c          //
                           ));
}

template <typename T>
Plotly::Trace edges(std::vector<std::pair<T, T>> &x,
                    std::vector<std::pair<T, T>> &y) {
  // x's size is used as the final size
  std::vector<PlotlyMsg::SeriesAny_value> edge_x, edge_y;
  edge_x.reserve(x.size());
  edge_y.reserve(x.size());
  for (uint i = 0; i < x.size(); ++i) {
    seriesAny_vector_push_back(edge_x, x[i].first);
    seriesAny_vector_push_back(edge_x, x[i].second);
    seriesAny_vector_push_back(edge_x, Plotly::NullValue);
    seriesAny_vector_push_back(edge_y, y[i].first);
    seriesAny_vector_push_back(edge_y, y[i].second);
    seriesAny_vector_push_back(edge_y, Plotly::NullValue);
  }
  return Plotly::Trace(PlotlyMsg::Trace::graph_objects, "Scatter",
                       Plotly::Dictionary(       //
                           "x", edge_x,          //
                           "y", edge_y,          //
                           "line_width", 0.5,    //
                           "line_color", "#888", //
                           "hoverinfo", "none",  //
                           "mode", "lines"       //
                           ));
}

template <typename T>
Plotly::Trace nodes(std::vector<T> &x, std::vector<T> &y) {
  return Plotly::Trace(PlotlyMsg::Trace::graph_objects, "Scatter",
                       Plotly::Dictionary(                              //
                           "x", x,                                      //
                           "y", y,                                      //
                           "mode", "markers",                           //
                           "hoverinfo", "text",                         //
                           "marker_showscale", true,                    //
                           "marker_colorscale", "YlGnBu",               //
                           "marker_reversescale", true,                 //
                           "marker_color", std::vector<int>(),          // why?
                           "marker_size", 10,                           //
                           "marker_colorbar_thickness", 15,             //
                           "marker_colorbar_title", "Node Connections", //
                           "marker_colorbar_xanchor", "left",           //
                           "marker_colorbar_titleside", "right",        //
                           "marker_line_width", 2                       //
                           ));
}

} // namespace TraceTemplate
} // namespace Plotly
