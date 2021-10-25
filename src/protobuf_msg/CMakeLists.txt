# +-----------------------------------------------------------------------------
# generate protobuf cpp
protobuf_generate_cpp(PROTO_SRC PROTO_HEADER msg.proto)
add_library(proto_plotly_cpp ${PROTO_HEADER} ${PROTO_SRC})
# python
protobuf_generate_python(PROTO_PY msg.proto)
add_custom_target(proto_plotly_py ALL DEPENDS ${PROTO_PY})
# add dependencies to auto build py m_msg when cpp m_msg updates
add_dependencies(proto_plotly_cpp proto_plotly_py)
# +-----------------------------------------------------------------------------

target_include_directories(
  proto_plotly_cpp PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

add_custom_command(
  TARGET proto_plotly_py
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/msg_pb2.py
          ${PYTHON_PKG_INSTALL_DIR}/cpp2py_plotly/msg_pb2.py)

install(
  TARGETS proto_plotly_cpp
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR}
  INCLUDES
  DESTINATION ${INSTALL_INCLUDE_DIR})

export(
  TARGETS proto_plotly_cpp
  APPEND
  FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
