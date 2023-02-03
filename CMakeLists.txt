cmake_minimum_required(VERSION 3.21.1)
cmake_policy(SET CMP0002 NEW)
include(ExternalProject)
project(DStream)

option(BUILD_DSTREAM_BENCHMARK	"Build the benchmark that compares different depth encoding algorithms" ON)
option(BUILD_DSTREAM_CMD		"Build the command line executable that encodes or decodes depthmaps" ON)

set(CMAKE_CXX_STANDARD 17)

# Build libjpeg
ExternalProject_Add(libjpeg-turbo
  URL https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/2.1.90.tar.gz
  URL_HASH SHA256=426d097a29bd67ab7ef8584673ba2e0da5fe65dcddd0c8daa123bdcb7a5e871f
  BUILD_COMMAND cmake ./.
)
link_directories(deps/libjpeg-turbo/Build/lib)

# LIB SOURCE
set (DSTREAM_LIB_SRC
	dstream-lib/src/Coder.cpp
	dstream-lib/src/Processing.cpp
	dstream-lib/src/AlgorithmImplementation.cpp
	dstream-lib/src/Implementations/Packed.cpp
	dstream-lib/src/Implementations/Hilbert.cpp
	
	dstream-lib/src/Coder.h
	dstream-lib/src/Vec3.h
	dstream-lib/src/Table.h
	dstream-lib/src/Processing.h
	dstream-lib/src/AlgorithmImplementation.h
	dstream-lib/src/Implementations/Packed.h
	dstream-lib/src/Implementations/Hilbert.h
)

set (DSTREAM_LIB_HEADERS
	dstream-lib/src/Coder.h
	dstream-lib/src/Processing.h
	
	dstream-lib/src/AlgorithmImplementation.h
	dstream-lib/src/Implementations/Packed.h
	dstream-lib/src/Implementations/Hilbert.h

	dstream-lib/src/Table.h
	dstream-lib/src/Vec3.h
)

# Add library
add_library(dstream-static STATIC ${DSTREAM_LIB_SRC})
target_link_libraries(dstream-static
	PUBLIC jpeg
)
target_include_directories (dstream-static
	PRIVATE	deps/libjpeg-turbo/Build/include
	PUBLIC ${DSTREAM_LIB_HEADERS}
)

# Add benchmark
if (BUILD_DSTREAM_BENCHMARK)
	set (DSTREAM_BENCHMARK_SRC
		dstream-benchmark/src/DepthmapReader.cpp
		dstream-benchmark/src/ImageWriter.cpp
		dstream-benchmark/src/Main.cpp
		dstream-benchmark/src/Timer.cpp
		
		dstream-benchmark/src/DepthmapReader.h
		dstream-benchmark/src/ImageWriter.h
		dstream-benchmark/src/Timer.h
	)

	set (DSTREAM_BENCHMARK_HEADERS
		dstream-benchmark/src/DepthmapReader.h
		dstream-benchmark/src/ImageWriter.h
		dstream-benchmark/src/Timer.h
	)
	
	add_executable(dstream-benchmark ${DSTREAM_BENCHMARK_SRC})
	target_link_libraries(dstream-benchmark
		PUBLIC dstream-static
	)
	target_include_directories (dstream-benchmark
		PUBLIC ${DSTREAM_LIB_HEADERS}
	)
endif()

# Add CMD executable
if (BUILD_DSTREAM_CMD)
	set (DSTREAM_CMD_SRC
		dstream-cmd/src/Main.cpp
	)
	
	add_executable(dstream-cmd ${DSTREAM_CMD_SRC})
	target_link_libraries(dstream-cmd
		PUBLIC dstream-static
	)
endif()

