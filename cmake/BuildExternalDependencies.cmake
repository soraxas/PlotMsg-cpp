cmake_minimum_required(VERSION 3.7)

include(FetchContent)

# check if the dependencies already exists
find_library(PROTOBUF_LIB protobuf)
find_library(ZMQ_LIB zmq)
find_package(cppzmq QUIET)

# ==================================================
if( "${CPP2PY_FORCE_BUILD_DEPS}" OR NOT PROTOBUF_LIB)
	set (EXT_LIBPROTOBUF "ext_libprotobuf")
	FetchContent_Declare (
		${EXT_LIBPROTOBUF}

		PREFIX         ${EXT_LIBPROTOBUF}
		GIT_REPOSITORY https://github.com/protocolbuffers/protobuf
		GIT_TAG        6aa539bf0195f188ff86efe6fb8bfa2b676cdd46 #v3.15.6
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
endif()
# ==================================================
if( "${CPP2PY_FORCE_BUILD_DEPS}" OR NOT ZMQ_LIB)
	set (EXT_LIBZMQ "ext_libzmq")
	set(BUILD_TESTS OFF CACHE INTERNAL "")  # Forces the value
	FetchContent_Declare (
		${EXT_LIBZMQ}

		PREFIX         ${EXT_LIBZMQ}
		GIT_REPOSITORY https://github.com/zeromq/libzmq
		GIT_TAG        4097855ddaaa65ed7b5e8cb86d143842a594eebd #v4.3.4
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
endif()
# ==================================================
if(NOT "${cppzmq_FOUND}")
	set (EXT_LIBCPPZMQ "ext_libcppzmq")
	set(CPPZMQ_BUILD_TESTS OFF CACHE INTERNAL "")  # Forces the value
	FetchContent_Declare (
		${EXT_LIBCPPZMQ}

		PREFIX         ${EXT_LIBCPPZMQ}
		GIT_REPOSITORY https://github.com/zeromq/cppzmq
		GIT_TAG        76bf169fd67b8e99c1b0e6490029d9cd5ef97666 #v4.7.1
		GIT_SHALLOW    ON

		BUILD_ALWAYS   OFF
		INSTALL_DIR    ${CMAKE_CURRENT_BINARY_DIR}/ext/${EXT_LIBCPPZMQ}

	#	# do not build as this is a header-only library
	#	CONFIGURE_COMMAND ""
	#	UPDATE_COMMAND    ""
	#	INSTALL_COMMAND   ""

	#	CMAKE_ARGS
		CMAKE_CACHE_ARGS
			-DCPPZMQ_BUILD_TESTS:BOOL=OFF
			-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>

	)
	# export the fetched binary folder as the lib folder for cppzmq
	# this is necessary to expose the `cppzmqConfig.cmake` file
	set(cppzmq_DIR ${cppzmq_BINARY_DIR})
endif()
# ==================================================
# fetch the actual content
FetchContent_MakeAvailable(${EXT_LIBZMQ} ${EXT_LIBCPPZMQ} ${EXT_LIBPROTOBUF})

