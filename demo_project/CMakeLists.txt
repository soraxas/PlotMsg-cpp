cmake_minimum_required(VERSION 3.7)
project(out_of_build_example)

# set(CPP2PY_FORCE_BUILD_DEPS true)  # uncomment this line to force build dependencies
# you can leave out the following line if you already had dependencies installed
include(../cmake/BuildExternalDependencies.cmake)


add_subdirectory(
    # you can replace the following with the cloned absolute path for this repo
    ..
    cpp2py_plotly
)
find_package( cpp2py_plotly REQUIRED )


add_executable(test "src/main.cpp")

get_directory_property(targets BUILDSYSTEM_TARGETS)
message("Targets: ${targets}")

target_link_libraries( test cpp2py_plotly )
