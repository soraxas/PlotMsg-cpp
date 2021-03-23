cmake_minimum_required(VERSION 3.5.1)
project(cpp2py_plotly)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 14)

# Set output directories for libraries and executables
set(BASE_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BASE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BASE_DIR}/bin)

# Installation locations
set(INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
set(INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include/cpp2py_plotly)
set(INSTALL_CMAKE_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/cpp2py_plotly)


# +-----------------------------------------------------------------------------
# | Build configuration options
# +-----------------------------------------------------------------------------
#option( BUILD_CTAGS "Build ctag file?" FALSE )
#option( BUILD_APPLICATIONS "Build applications?" FALSE )

# +-----------------------------------------------------------------------------
# | Library search and setup
# +-----------------------------------------------------------------------------

# Build shared libraries by default
set(BUILD_SHARED_LIBS True)

find_package(ZMQ)
find_package(Protobuf)

# Use local include and library directories
include_directories(
        ${ZMQ_INCLUDE_DIRS}
        ${Protobuf_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}
)
link_directories(${BASE_DIR}/lib)

# +-----------------------------------------------------------------------------
# | Custom targets and macros
# +-----------------------------------------------------------------------------

if (BUILD_CTAGS)
    # Custom target that builds ctags for the source code
    add_custom_target(
            generate_ctags ALL
            ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .
            WORKING_DIRECTORY ${BASE_DIR}/src
            COMMENT "Regenerating ctags file"
    )
endif ()


# +-----------------------------------------------------------------------------
# | Compiler settings
# +-----------------------------------------------------------------------------
# Compiler specifiic settings to enable C++14 support
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -pthread")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wno-int-in-bool-context -pthread")
endif ()


# +-----------------------------------------------------------------------------
# | Compile code
# +-----------------------------------------------------------------------------

add_subdirectory(src/protobuf_msg)
add_subdirectory(src/cpp2py_plotly)

# python scripts
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cpp2py_plotly.py
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cpp2py_plotly.py
)

# demo
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # only build demo if this is the root project
    add_executable(pub "publisher_example.cpp")
    target_link_libraries(pub cpp2py_plotly)
endif ()

# +-----------------------------------------------------------------------------
# | Installation setup
# +-----------------------------------------------------------------------------

# Export the package for use from the build tree. This
# registers the build tree with a global CMake registry.
export(PACKAGE ${PROJECT_NAME})

# Determine the relative path between CMake configuration file installation
# location and header files installation directory
file(RELATIVE_PATH REL_INCLUDE_DIR ${INSTALL_CMAKE_DIR} ${INSTALL_INCLUDE_DIR})

# Create the ${ProjectName}Config.cmake file for the build tree.
set(CONF_INCLUDE_DIRS ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
configure_file(
        ${PROJECT_NAME}Config.cmake.in
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY
)

# Create the ${PROJECT_NAME}Config.cmake file for the install tree.
# CONF_INCLUDE_DIRS is a variable that the generated
# ${PROJECT_NAME}Config.cmake file uses by performing variable substitution on.
# This is the reason that ${*_CMAKE_DIR} is used even though it is not defined
# here.
set(CONF_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
configure_file(
        ${PROJECT_NAME}Config.cmake.in
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake" @ONLY
)

# Install the ${PROJECT_NAME}Config.cmake file
install(
        FILES "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}"
        COMPONENT dev
)

# Install the export set for use with the install tree
install(
        EXPORT ${PROJECT_NAME}Targets
        DESTINATION "${INSTALL_CMAKE_DIR}"
        COMPONENT dev
)


#function(combine_archives output_archive list_of_input_archives)
#    set(mri_file ${TEMP_DIR}/${output_archive}.mri)
#    set(FULL_OUTPUT_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${output_archive}.a)
#    file(WRITE ${mri_file} "create ${FULL_OUTPUT_PATH}\n")
#    FOREACH (in_archive ${list_of_input_archives})
#        file(APPEND ${mri_file} "addlib ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${in_archive}.a\n")
#    ENDFOREACH ()
#    file(APPEND ${mri_file} "save\n")
#    file(APPEND ${mri_file} "end\n")
#
#    set(output_archive_dummy_file ${TEMP_DIR}/${output_archive}.dummy.cpp)
#    add_custom_command(OUTPUT ${output_archive_dummy_file}
#            COMMAND touch ${output_archive_dummy_file}
#            DEPENDS ${list_of_input_archives})
#
#    add_library(${output_archive} STATIC ${output_archive_dummy_file})
##    add_custom_command(TARGET ${output_archive}
##            POST_BUILD
##            COMMAND ar -M < ${mri_file})
#endfunction(combine_archives)
#
#set(TEMP_DIR /tmp)
#set(target_combine_lib
#        cpp2py_plotly
#        proto_plotly_cpp
#        ${Protobuf_LIBRARIES}
##        zmq
#        ${ZMQ_LIBRARIES}
#        )
#
#combine_archives(cpp2py_plotly_Combined
#        "${target_combine_lib}"
#        )