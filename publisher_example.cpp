#include <iostream>
#include <numeric>

#include "plotmsg/plotmsg.hpp"
#include "plotmsg/template/core.hpp"

#ifdef WITH_EIGEN
#include <Eigen/Dense>
#endif

int main(int argc, char *argv[]) {

  PlotMsg::Dictionary d("a", 3);

  d["24"] = 4;
  d["24"] = "lollll";
  d["24"] = 1.11;
  d["2v4"] = "lollll";
  d["2v4"] = PlotMsg::Dictionary("x", "xxx", "y", 999);

  std::cout << d << std::endl;

  std::vector<double> dou = {1, 2.34235, 346.4, 73.4};

  //  d["2v4"]["xy"]["fa"]["1"]["4"]["92"]["0129"]["89982"] = dou;
  //  d["2v4"]["x"] = 89;

  //  std::cout << d["24"] << std::endl;

  std::cout << d << std::endl;

  PlotMsg::Figure fig22;

  fig22._add_trace();
  //  fig22.trace(-1).m_kwargs["2"]["hahhaa"] = 34;
  fig22.trace(-1)["2v4"]["xy"]["fa"]["1"]["4"]["92"]["0129"]["89982"] = dou;

  fig22.send();

  return 0;

  auto x = std::vector<double>(20, 1);
  auto y = std::vector<double>(20, 4.2);
  auto o = std::vector<int>(20, 7);

  std::cout << PlotMsg::TraceTemplate::scatter() << std::endl;
  std::cout << PlotMsg::TraceTemplate::scatter(x, y) << std::endl;
  std::cout << PlotMsg::TraceTemplate::scatter_with_colour(x, y, o)
            << std::endl;

  std::iota(std::begin(x), std::end(x), 0);
  std::iota(std::begin(y), std::end(y), 5);
  std::iota(std::begin(o), std::end(o), 3);

  using ditem = PlotMsg::Dictionary::DictionaryItemPair;

  PlotMsg::initialise_publisher();

  PlotMsg::Figure fig;

  fig.set_uuid("test_1");
  fig.add_trace(                                                   //
      PlotMsg::Trace(                                              //
          PlotMsgProto::PlotlyTrace::graph_objects, "Scatter",     //
          PlotMsg::Dictionary(                                     //
              ditem("mode", "markers"),                            //
              ditem("x", x),                                       //
              ditem("y", y),                                       //
              ditem("marker",                                      //
                    PlotMsg::Dictionary(                           //
                        ditem("size", 14),                         //
                        ditem("opacity", 0.5),                     //
                        ditem("colorscale", "Viridis"),            //
                        ditem("colorbar", PlotMsg::Dictionary(     //
                                              "title", "Colorbar") //
                              ),                                   //
                        ditem("color", o)                          //
                        )                                          //
                    )                                              //
              )));

  std::cout << fig.get_trace_copy(0) << std::endl;

  fig.add_command("update_yaxes",
                  PlotMsg::Dictionary("scaleanchor", "x", "scaleratio", 1));

  fig.send();
  fig.reset();

  {
    fig.set_uuid("graph");

    std::vector<double> __x = {1.24, 2.23, 5.23, 5.25, 1.34};
    std::vector<double> __y = {1.24, 2.23, 5.23, 5.25, 1.34};

    std::vector<std::pair<double, double>> ex = {
        {4.4, 9.4}, {4.4, -3.4}, {4.4, 3.7}};

    std::vector<std::pair<double, double>> ey = {
        {14.4, 23.4}, {14.4, 8.4}, {14.4, 3.7}};

    fig.add_trace(PlotMsg::TraceTemplate::vertices(__x, __y));
    fig.add_trace(PlotMsg::TraceTemplate::edges(ex, ey));

    fig.send();
  }

#ifdef WITH_EIGEN

  auto X = Eigen::RowVectorXd::LinSpaced(3, 1, 3).replicate(5, 1);
  auto Y = Eigen::VectorXd::LinSpaced(5, 10, 14).replicate(1, 3);

  //  std::vector<double> x = {};
  //
  //  fig.add_trace();

  //  x ,y = np.meshgrid(np.arange(0, 4, .2), np.arange(0, 4, .2))
  // u = np.cos(x)*y
  // v = np.sin(x)*y
  //
  // f = ff.create_quiver(x, y, u, v)
  // trace1 = f.data[0]
  // trace2 = go.Contour(
  //       z=[[10, 10.625, 12.5, 15.625, 20],
  //          [5.625, 6.25, 8.125, 11.25, 15.625],
  //          [2.5, 3.125, 5., 8.125, 12.5],
  //          [0.625, 1.25, 3.125, 6.25, 10.625],
  //          [0, 0.625, 2.5, 5.625, 10]]
  //   )
  // data=[trace1,trace2]
  // fig = go.FigureWidget(data)
  // fig
  //
  // f = ff.create_quiver(x, y, u, v)
  // trace1 = f.data[0]
  // trace2 = go.Contour(
  //       z=[[10, 10.625, 12.5, 15.625, 20],
  //          [5.625, 6.25, 8.125, 11.25, 15.625],
  //          [2.5, 3.125, 5., 8.125, 12.5],
  //          [0.625, 1.25, 3.125, 6.25, 10.625],
  //          [0, 0.625, 2.5, 5.625, 10]]
  //   )
  // data=[trace1,trace2]
  // fig = go.FigureWidget(data)
  // fig

#endif

  std::cout << "pub done" << std::endl;
  return 0;
}
