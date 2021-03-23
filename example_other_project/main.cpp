#include "cpp2py_plotly.hpp"

int main(int argc, char const *argv[]) {
  Plotly::Figure fig;
  std::vector<double> x{1, 2, 3, 4, 5, 6, 7};
  std::vector<double> y{1, 2, 3, 2, 3, 4, 5};

  fig.add_kwargs("x", x);
  fig.add_kwargs("y", y);

  fig.send();
}

