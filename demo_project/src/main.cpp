#include "plotmsg/main.hpp"

int main(int argc, char const *argv[]) {


  PlotMsg::Figure fig("PlotMsg demo");
  std::vector<double> x{1, 2, 3, 4, 5, 6, 7};
  std::vector<double> y{1, 2, 3, 2, 3, 4, 5};

  auto trace = PlotMsg::Trace( //
      PlotMsg::PlotlyTrace::graph_objects, "Scatter",
      PlotMsg::Dictionary(   //
          "mode", "markers", //
          "marker_size", 10, //
          "marker_colorscale", "Viridis",     //
          "marker_colorbar_title", "Colorbar",//
          "x", x,                             //
          "y", y                              //
          )                                   //
  );

  /*
   * OR you can use the template to simplify the procedure

  #include "plotmsg/template/core.hpp"

  auto trace = PlotMsg::TraceTemplate::scatter(x, y);
  // trace["mode"] = "lines";

  */

  fig.add_trace(trace);

  fig.send();

  std::cout << "Done" << std::endl;
}

