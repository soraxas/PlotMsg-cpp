#include <iostream>
#include <chrono>
#include <thread>

#include <build/msg.pb.h>

#include <utility>
#include <zmq.hpp>

#include "src/cpp2py_plotly.hpp"

int main(int argc, char *argv[]) {

    Plotly::initialise_publisher();
    Plotly::Figure fig;


    fig.add_kwargs("x", std::vector<double>(3, 2));
    fig.add_kwargs("y", std::vector<int>(7, 4));
    fig.add_kwargs("z", std::vector<double>(2, 2.3223));


    fig.add_kwargs("type", "lol");


    Plotly::Dictionary dict;


    auto aaa = std::vector<int>({1, 2, 3, 4, 5});
    dict.add_kwargs("x", aaa);
    dict.add_kwargs("oh no!", 2.4123);
    dict.add_kwargs("oho!", false);


    fig.add_kwargs("vvvvvvvvvvvv", dict);


    fig.send(zmq::send_flags::none);
    std::cout << "hi" << std::endl;


    std::cout << "pub done" << std::endl;
    return 0;
}
