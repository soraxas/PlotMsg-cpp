#include <iostream>

#include "cpp2py_plotly.hpp"

int main(int argc, char *argv[]) {

  auto x = std::vector<double>(20, 1);
  auto y = std::vector<double>(20, 4.2);
  auto o = std::vector<int>(20, 7);

  using ditem = Plotly::Dictionary::DictionaryItemPair;

  Plotly::initialise_publisher();

  Plotly::Figure fig;

  fig.set_uuid("test_1");
  fig.set_kwargs(                  //
      Plotly::Dictionary(          //
          ditem("mode", "marker"), //
          ditem("x", x),           //
          ditem("y", y),           //
          ditem("markers",
                Plotly::Dictionary(                                    //
                    ditem("size", 20),                                 //
                    ditem("opacity", 0.5),                             //
                    ditem("colorscale", "Viridis"),                    //
                    ditem("colorbar", Plotly::Dictionary("title",      //
                                                         "Colorbar")), //
                    ditem("color", o)                                  //
                    )                                                  //
                )                                                      //
          ));
  std::cout << fig.get_kwargs() << std::endl;
  fig.send();

  Plotly::Dictionary dict;

  auto aaa = std::vector<int>({1, 2, 3, 4, 5});
  dict.add_kwargs("x", aaa);
  dict.add_kwargs("oh no!", 2.4123);
  dict.add_kwargs("oho!", false);

  fig.add_kwargs("vvvvvvvvvvvv", dict);

  fig.send(zmq::send_flags::none);

  std::cout << "pub done" << std::endl;
  return 0;
}
