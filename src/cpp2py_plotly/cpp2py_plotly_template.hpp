#pragma once

#include "cpp2py_plotly.hpp"

/*
 * Implements tempalte message for easy usage
 */

namespace Plotly {
namespace FigTemplate {
using ditem = Plotly::Dictionary::DictionaryItemPair;

Plotly::Figure Scatter() {
  Plotly::Figure fig;

  fig.set_uuid("scatter");
  fig.add_trace_by_kwargs(          //
      Plotly::Dictionary(           //
          ditem("mode", "markers"), //
          ditem("marker",
                Plotly::Dictionary(                            //
                    ditem("size", 10),                         //
                    ditem("opacity", 0.9),                     //
                    ditem("colorscale", "Viridis"),            //
                    ditem("colorbar", Plotly::Dictionary(      //
                                          "title", "Colorbar") //
                          )                                    //
                    )                                          //
                )                                              //
          ));
  return fig;
}

template <typename T1, typename T2>
Plotly::Figure Scatter(std::vector<T1> x, std::vector<T2> y) {
  Plotly::Figure fig = Scatter();
  fig.add_kwargs_to_trace(0, "x", x);
  fig.add_kwargs_to_trace(0, "y", y);
  return fig;
}

template <typename T1, typename T2, typename T3>
Plotly::Figure ScatterWithColour(std::vector<T1> x, std::vector<T2> y,
                                 std::vector<T3> c) {
  Plotly::Figure fig = Scatter(x, y);
  fig.add_kwargs_to_trace(0, "color", c);
  return fig;
}

} // namespace FigTemplate
} // namespace Plotly