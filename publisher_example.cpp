#include <iostream>
#include <numeric>

#include "cpp2py_plotly.hpp"

int main(int argc, char *argv[]) {

  auto x = std::vector<double>(20, 1);
  auto y = std::vector<double>(20, 4.2);
  auto o = std::vector<int>(20, 7);

  std::iota(std::begin(x), std::end(x), 0);
  std::iota(std::begin(y), std::end(y), 5);
  std::iota(std::begin(o), std::end(o), 3);

  using ditem = Plotly::Dictionary::DictionaryItemPair;

  Plotly::initialise_publisher();

  Plotly::Figure fig;

  fig.set_uuid("test_1");
  fig.set_kwargs(                   //
      Plotly::Dictionary(           //
          ditem("mode", "markers"), //
          ditem("x", x),            //
          ditem("y", y),            //
          ditem("marker",
                Plotly::Dictionary(                            //
                    ditem("size", 14),                         //
                    ditem("opacity", 0.5),                     //
                    ditem("colorscale", "Viridis"),            //
                    ditem("colorbar", Plotly::Dictionary(      //
                                          "title", "Colorbar") //
                          ),                                   //
                    ditem("color", o)                          //
                    )                                          //
                )                                              //
          ));

  std::cout << fig.get_dict_copy() << std::endl;
  fig.send();

  Plotly::Dictionary dict;

  //  auto aaa = std::vector<int>({1, 2, 3, 4, 5});
  //  dict.add_kwargs("x", aaa);
  //  dict.add_kwargs("oh no!", 2.4123);
  //  dict.add_kwargs("oho!", false);
  //
  //  fig.add_kwargs("vvvvvvvvvvvv", dict);
  //  fig.send(zmq::send_flags::none);

  std::cout << "pub done" << std::endl;
  return 0;
}
