# Define lists of files
set(SOURCE_FILES plotmsg/plotmsg.cpp)
set(HEADER_FILES plotmsg/plotmsg.hpp plotmsg/template/core.hpp
                 plotmsg/template/ompl .hpp)
set(LINK_LIBARARIES proto_plotmsg_cpp ${Protobuf_LIBRARIES} zmq
                    ${ZMQ_LIBRARIES})

# Build library
add_library(plotmsg ${SOURCE_FILES})
target_link_libraries(plotmsg ${LINK_LIBARARIES})

target_include_directories(
  plotmsg PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                 $<INSTALL_INTERFACE:> # <prefix>/include/mylib
)

# Installation setup
set_target_properties(
  plotmsg PROPERTIES PUBLIC_HEADER "${HEADER_FILES}"
                     IMPORTED_LINK_DEPENDENT_LIBRARIES "${LINK_LIBARARIES}")

install(
  TARGETS plotmsg
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR}
  INCLUDES
  DESTINATION ${INSTALL_INCLUDE_DIR})

# Add all targets to the build-tree export set
export(
  TARGETS plotmsg
  APPEND
  FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
