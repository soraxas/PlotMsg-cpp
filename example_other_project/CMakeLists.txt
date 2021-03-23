cmake_minimum_required(VERSION 3.5.1)
set(PROJECT_NAME out_of_build_example)

add_subdirectory(
        ..
        cpp2py_plotly
)
find_package( cpp2py_plotly REQUIRED )

add_executable(test "main.cpp")

target_link_libraries( test cpp2py_plotly )

