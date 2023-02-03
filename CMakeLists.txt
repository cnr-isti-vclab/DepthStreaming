cmake_minimum_required(VERSION 3.21.1)
cmake_policy(SET CMP0002 NEW)
include(ExternalProject)
project(DStream)

option(BUILD_DSTREAM_BENCHMARK	"Build the benchmark that compares different depth encoding algorithms" ON)
option(BUILD_DSTREAM_CMD		"Build the command line executable that encodes or decodes depthmaps" ON)

set(CMAKE_CXX_STANDARD 17)

# Build libjpeg
ExternalProject_Add(libjpeg-turbo
  URL https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/2.0.6/libjpeg-turbo-2.0.6.tar.gz
  URL_HASH SHA256=b0c1b8b7cee0a91d11e0b1a8b9d9f8716db5ffa4f085b6f1ab6ee4e3e1af2c51
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=deps/libjpeg-turbo/Build
  BUILD_COMMAND make
  INSTALL_COMMAND make install
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

