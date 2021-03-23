# Plotly for C++ via Python

This repository brings **Plotly** to C++ by utilising the native Plotly library in Python. This is achieved by implementing a *publisher* and *subscriber* model between C++ and Python, and a stateful model in the **Jupyter notebook** ecosystem that captures the incoming message. It supports asynchronous refresh within the notebook whenever a new message is published from your C++ files, and automatically display them within the notebook. This library also support non-notebook usage where it uses Plotly's native figure display from within your browser.

# Requirements

- zeromq (messaging protocol)

- protobuf (serialisation of message)

# Using the library in your C++ project

You can include the project it as you normally. The following shows a snippet to quickly visualise your code using Plotly without much setup

## Quick and dirty out-of-tree build in cmake

In your **CMakeLists.txt**:

```cmake
cmake_minimum_required(...)

...

# Add as out-of-tree build (with absolute path & binary directory)
add_subdirectory(
        /home/soraxas/git-repo/cpp2py_plotly
        cpp2py_plotly
)
find_package( cpp2py_plotly REQUIRED )

...

# link the library to your target
target_link_libraries( ${MY_TARGET} cpp2py_plotly )
```

# Example Usage

Say you have something like this in your `${MY_TARGET}`

```cpp
#include "cpp2py_plotly.hpp"

int main(int argc, char const *argv[]) {
  Plotly::Figure fig;
  std::vector<double> x{1, 2, 3, 4, 5, 6, 7};
  std::vector<double> y{1, 2, 3, 2, 3, 4, 5};

  fig.add_kwargs("x", x);
  fig.add_kwargs("y", y);

  fig.send();
}
```

Run

```shell
cmake -Bbuild
make -C build
```

to build your C++ project. 

You can then, for example, start the python subscriber with

```sh
ipython -i lib/cpp2py_plotly.py
```

```python
sub = Cpp2PyPlotly()
sub.initialise()
sub.spin()
```

Afterwards, the ipython kernel is listening at the *default* **tcp://127.0.0.1:5557** socket and, when you execute your compiled C++ binary, it will send the `fig` message using the same socket.

## Example Project

`./example_other_project` is an example of a simple project. You can test it out with

```sh
cd example_other_project
cmake -Bbuild
make -C build

# start subscriber in the background (1)
python py_sub_example.py &

# publish figure from c++
./build/test

# killall background jobs (py script in (1) that is running in bg )
kill $(jobs -p)
```
