cmake_minimum_required(VERSION 3.7)

include(FetchContent)

# ==================================================
set (EXT_LIBPROTOBUF "ext_libprotobuf")
FetchContent_Declare (
	${EXT_LIBPROTOBUF}

	PREFIX         ${EXT_LIBPROTOBUF}
	GIT_REPOSITORY https://github.com/protocolbuffers/protobuf
	GIT_TAG        v3.15.6
	GIT_SHALLOW    ON
	SOURCE_SUBDIR  cmake

	BUILD_ALWAYS   OFF
	INSTALL_DIR    ${CMAKE_CURRENT_BINARY_DIR}/ext/${EXT_LIBPROTOBUF}

	CMAKE_ARGS
		-DBUILD_TESTS=OFF
	CMAKE_CACHE_ARGS
		-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>

#	BUILD_COMMAND     ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release --target INSTALL
)
# ==================================================
set (EXT_LIBZMQ "ext_libzmq")
set(BUILD_TESTS OFF CACHE INTERNAL "")  # Forces the value
FetchContent_Declare (
	${EXT_LIBZMQ}

	PREFIX         ${EXT_LIBZMQ}
	GIT_REPOSITORY https://github.com/zeromq/libzmq
	GIT_TAG        v4.3.4
	GIT_SHALLOW    ON

	BUILD_ALWAYS   OFF
	INSTALL_DIR    ${CMAKE_CURRENT_BINARY_DIR}/ext/${EXT_LIBZMQ}

	CMAKE_ARGS
  -DBUILD_TESTS:BOOL=OFF

	CMAKE_CACHE_ARGS
  -DBUILD_TESTS:BOOL=OFF
#		-DBUILD_SHARED_LIBS:BOOL=ON
#		-DENABLE_STATIC_RUNTIME:BOOL=OFF
#		-DBUILD_EXAMPLES:BOOL=ON
		-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>

#	BUILD_COMMAND     ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release --target INSTALL
)
# ==================================================
set (EXT_LIBCPPZMQ "ext_libcppzmq")
set(CPPZMQ_BUILD_TESTS OFF CACHE INTERNAL "")  # Forces the value
FetchContent_Declare (
	${EXT_LIBCPPZMQ}

	PREFIX         ${EXT_LIBCPPZMQ}
	GIT_REPOSITORY https://github.com/zeromq/cppzmq
	GIT_TAG        v4.7.1
	GIT_SHALLOW    ON

#	# do not build as this is a header-only library
#	CONFIGURE_COMMAND ""
#	UPDATE_COMMAND    ""
#	INSTALL_COMMAND   ""

#	CMAKE_ARGS
	CMAKE_CACHE_ARGS
		-DCPPZMQ_BUILD_TESTS:BOOL=OFF
		-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>

	BUILD_ALWAYS   OFF
	INSTALL_DIR    ${CMAKE_CURRENT_BINARY_DIR}/ext/${EXT_LIBCPPZMQ}
)
# ==================================================
# fetch the actual content
FetchContent_MakeAvailable(${EXT_LIBZMQ} ${EXT_LIBCPPZMQ} ${EXT_LIBPROTOBUF})

