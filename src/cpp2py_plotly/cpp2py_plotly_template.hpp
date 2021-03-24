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
      Plotly::Dictionary(    //
          "mode", "markers", //
          "marker",
          Plotly::Dictionary(          //
              "size", 10,              //
              "opacity", 0.9,          //
              "colorscale", "Viridis", //
              "colorbar",
              Plotly::Dictionary(     //
                  "title", "Colorbar" //
                  )                   //
              )                       //
          )                           //
  );
}

template <typename T1, typename T2>
Plotly::Trace Scatter(std::vector<T1> x, std::vector<T2> y) {
  Plotly::Trace trace = Scatter();
  trace.m_kwargs.add_kwargs("x", x);
  trace.m_kwargs.add_kwargs("y", y);
  return trace;
}

template <typename T1, typename T2>
Plotly::Trace ScatterWithColour(std::vector<T1> x, std::vector<T1> y,
                                std::vector<T2> c) {
  Plotly::Trace trace = Scatter(x, y);
  trace.m_kwargs.add_kwargs("color", c);
  return trace;
}

template <typename T>
Plotly::Trace vector_field(std::vector<T> x, std::vector<T> y, std::vector<T> u,
                           std::vector<T> v) {
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
Plotly::Trace contour(std::vector<T> x, std::vector<T> y, std::vector<T> c) {
  return Plotly::Trace(PlotlyMsg::Trace::graph_objects, "Contour",
                       Plotly::Dictionary( //
                           "x", x,         //
                           "y", y,         //
                           "z", c          //
                           ));
}

} // namespace TraceTemplate
} // namespace Plotly
