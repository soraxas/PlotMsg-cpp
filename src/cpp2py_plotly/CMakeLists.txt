# Define lists of files
set(SOURCE_FILES cpp2py_plotly.cpp)
set(HEADER_FILES cpp2py_plotly.hpp cpp2py_plotly_template.hpp
                 cpp2py_plotly_ompl.hpp)
set(LINK_LIBARARIES proto_plotly_cpp ${Protobuf_LIBRARIES} zmq ${ZMQ_LIBRARIES})

# Build library
add_library(cpp2py_plotly ${SOURCE_FILES})
target_link_libraries(cpp2py_plotly ${LINK_LIBARARIES})

target_include_directories(
  cpp2py_plotly PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                       $<INSTALL_INTERFACE:> # <prefix>/include/mylib
)

# Installation setup
set_target_properties(
  cpp2py_plotly
  PROPERTIES PUBLIC_HEADER "${HEADER_FILES}" IMPORTED_LINK_DEPENDENT_LIBRARIES
                                             "${LINK_LIBARARIES}")

install(
  TARGETS cpp2py_plotly
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR}
  INCLUDES
  DESTINATION ${INSTALL_INCLUDE_DIR})

# Add all targets to the build-tree export set
export(
  TARGETS cpp2py_plotly
  APPEND
  FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
