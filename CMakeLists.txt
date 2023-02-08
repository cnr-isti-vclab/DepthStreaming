cmake_minimum_required(VERSION 3.21.1)
cmake_policy(SET CMP0002 NEW)
include(ExternalProject)
project(DStream)

option(BUILD_DSTREAM_BENCHMARK	"Build the benchmark that compares different depth encoding algorithms" ON)
option(BUILD_DSTREAM_CMD		"Build the command line executable that encodes or decodes depthmaps" ON)

set(CMAKE_CXX_STANDARD 17)
set (EXTERNAL_DEPS	${CMAKE_BINARY_DIR}/../ext/)

# Build libjpeg
ExternalProject_Add(libjpeg-turbo
	PREFIX libjpeg-turbo
	BINARY_DIR ${EXTERNAL_DEPS}/libjpeg-turbo
	URL https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/2.1.90.tar.gz
	URL_HASH SHA256=426d097a29bd67ab7ef8584673ba2e0da5fe65dcddd0c8daa123bdcb7a5e871f
	BUILD_COMMAND cmake ./.
)

# LIB SOURCE
set (DSTREAM_LIB_SRC
	lib/StreamCoder.cpp
	lib/Coder.cpp
	lib/DepthProcessing.cpp
	
	lib/Implementations/Packed.cpp
	lib/Implementations/Hilbert.cpp
	lib/Implementations/Morton.cpp
	lib/Implementations/Hue.cpp
	lib/Implementations/Split.cpp
	lib/Implementations/Triangle.cpp
	lib/Implementations/Phase.cpp

	
	lib/StreamCoder.h
	lib/DataStructs/Vec3.h
	lib/DataStructs/Table.h
	lib/DepthProcessing.h
	lib/Coder.h
	lib/Implementations/Packed.h
	lib/Implementations/Hilbert.h
	lib/Implementations/Morton.h
	lib/Implementations/Hue.h
	lib/Implementations/Split.h
	lib/Implementations/Triangle.h
	lib/Implementations/Phase.h
)

set (DSTREAM_LIB_HEADERS
	lib/StreamCoder.h
	lib/DepthProcessing.h
	lib/Coder.h
	
	lib/Implementations/Packed.h
	lib/Implementations/Hilbert.h
	lib/Implementations/Morton.h

	lib/DataStructs/Table.h
	lib/DataStructs/Vec3.h
)

# Add library
link_directories(
	${EXTERNAL_DEPS}/libjpeg-turbo
	Build
)
add_library(dstream-static STATIC ${DSTREAM_LIB_SRC})
target_link_libraries(dstream-static
	PUBLIC turbojpeg-static
)
target_include_directories (dstream-static
	PUBLIC lib
#	PUBLIC lib/Implementations
#	PUBLIC lib/Processing
#	PUBLIC lib/DataStructs
	PUBLIC ${CMAKE_BINARY_DIR}/libjpeg-turbo/src/libjpeg-turbo/
	PUBLIC ${EXTERNAL_DEPS}/libjpeg-turbo
)

# Add benchmark
if (BUILD_DSTREAM_BENCHMARK)
	set (DSTREAM_BENCHMARK_SRC
		benchmark/DepthmapReader.cpp
		benchmark/ImageWriter.cpp
		benchmark/Main.cpp
		benchmark/Timer.cpp
		benchmark/JpegEncoder.cpp
		benchmark/JpegDecoder.cpp
		benchmark/StbImport.cpp
		
		benchmark/DepthmapReader.h
		benchmark/ImageWriter.h
		benchmark/Timer.h
		
		benchmark/JpegEncoder.h
		benchmark/JpegDecoder.h
		benchmark/stb_image.h
		benchmark/stb_image_write.h
	)

	set (DSTREAM_BENCHMARK_HEADERS
		benchmark/DepthmapReader.h
		benchmark/ImageWriter.h
		benchmark/Timer.h
		benchmark/JpegEncoder.h
		benchmark/JpegDecoder.h
		benchmark/stb_image.h
		benchmark/stb_image_write.h
	)
	
	add_executable(dstream-benchmark ${DSTREAM_BENCHMARK_SRC})
	target_include_directories (dstream-benchmark
		PRIVATE benchmark
	)
	target_link_libraries(dstream-benchmark
		PUBLIC dstream-static
	)
endif()

# Add CMD executable
if (BUILD_DSTREAM_CMD)
	set (DSTREAM_CMD_SRC
		cmd/Main.cpp
	)
	
	add_executable(dstream-cmd ${DSTREAM_CMD_SRC})
	target_include_directories (dstream-cmd
		PRIVATE cmd
	)
	target_link_libraries(dstream-cmd
		PUBLIC dstream-static
	)
endif()

