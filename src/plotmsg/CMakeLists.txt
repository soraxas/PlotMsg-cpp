# Define lists of files
set(SOURCE_FILES plotmsg/plotmsg.cpp)
set(HEADER_FILES
    plotmsg/main.hpp
    plotmsg/_impl/core.hpp
    plotmsg/_impl/dictionary.hpp
    plotmsg/_impl/figure.hpp
    plotmsg/_impl/trace.hpp
    plotmsg/_impl/series_any.hpp
    plotmsg/_impl/index_proxy_access.hpp
    plotmsg/_impl/helpers.hpp
    plotmsg/template/core.hpp
    plotmsg/template/ompl.hpp)
set(LINK_LIBARARIES proto_plotmsg_cpp ${Protobuf_LIBRARIES} zmq
                    ${ZMQ_LIBRARIES})

# Build library
add_library(plotmsg ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(plotmsg ${LINK_LIBARARIES})

target_include_directories(
  plotmsg PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                 $<INSTALL_INTERFACE:> # <prefix>/include/mylib
)

# Installation setup
set_target_properties(
  plotmsg PROPERTIES #PUBLIC_HEADER "${HEADER_FILES}"
                     IMPORTED_LINK_DEPENDENT_LIBRARIES "${LINK_LIBARARIES}")

install(
  TARGETS plotmsg
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${INSTALL_LIB_DIR}
  PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR}
  INCLUDES
  DESTINATION ${INSTALL_INCLUDE_DIR})

install(DIRECTORY "plotmsg/" # source directory
        DESTINATION ${INSTALL_INCLUDE_DIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.hpp" # select header files
)

# install the generated protobuf header
get_target_property(PROTO_HEADER proto_plotmsg_cpp PUBLIC_HEADER)
install(FILES ${PROTO_HEADER}
        DESTINATION ${INSTALL_INCLUDE_DIR}/_impl
)

# message(FATAL_ERROR ${PROTO_HEADER} )

# Add all targets to the build-tree export set
export(
  TARGETS plotmsg
  APPEND
  FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
