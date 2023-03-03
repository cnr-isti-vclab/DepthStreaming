cmake_minimum_required(VERSION 3.19.1)
cmake_policy(SET CMP0002 NEW)
cmake_policy(SET CMP0077 NEW)
include(ExternalProject)
project(DStream)

option(BUILD_DSTREAM_BENCHMARK	"Build the benchmark that compares different depth encoding algorithms" ON)
option(BUILD_DSTREAM_CMD		"Build the command line executable that encodes or decodes depthmaps" ON)
option(ENABLE_PNG				"Build the project with libpng support. When enabled, zlib will also be linked. When disabled, \
									stb_image will be used to handle PNGs instead and zlib won't be linked" OFF)
option(ENABLE_WEBP				"Build the project with libwebp support. When enabled, libpng will also be linked" ON)
option(ENABLE_TIFF				"Build the project with libtiff support" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_POSTFIX d)
set (EXTERNAL_DEPS	${CMAKE_BINARY_DIR}/../ext)

# General dependency configuration
set(BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(ENABLED_SHARED OFF CACHE INTERNAL "" FORCE)
set(SKIP_INSTALL_ALL ON CACHE INTERNAL "" FORCE)

if(MSVC)
	# Build libjpeg
	ExternalProject_Add(libjpeg-turbo
		PREFIX libjpeg-turbo
		BINARY_DIR ${EXTERNAL_DEPS}/libjpeg-turbo
		URL https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/2.1.90.tar.gz
		URL_HASH SHA256=426d097a29bd67ab7ef8584673ba2e0da5fe65dcddd0c8daa123bdcb7a5e871f
	)
	
	if (${ENABLE_WEBP} OR ${ENABLE_PNG} OR ${ENABLE_TIFF})
		add_subdirectory(lib/vendor/zlib)
		configure_file("${PROJECT_SOURCE_DIR}/lib/vendor/zlib/zconf.h.in" "${PROJECT_SOURCE_DIR}/lib/vendor/zlib/zconf.h")
	endif()

	# Build libpng
	if (${ENABLE_WEBP} OR ${ENABLE_PNG})
		set(PNG_BUILD_ZLIB ON CACHE INTERNAL "" FORCE)
		set(PNG_SHARED OFF CACHE INTERNAL "" FORCE)
		set(PNG_EXECUTABLES OFF CACHE INTERNAL "" FORCE)
		set(PNG_TESTS OFF CACHE INTERNAL "" FORCE)
	
		add_subdirectory(lib/vendor/libpng)
		
		target_include_directories(png_static PRIVATE lib/vendor/zlib)
		target_link_libraries(png_static PRIVATE zlibstatic)
		
		configure_file("${PROJECT_SOURCE_DIR}/lib/vendor/libpng/scripts/pnglibconf.h.prebuilt" "${PROJECT_SOURCE_DIR}/lib/vendor/libpng/pnglibconf.h")
	endif()
	
	# Build libwebp
	if (${ENABLE_WEBP})
		set(WEBP_BUILD_CWEBP OFF CACHE INTERNAL "" FORCE)
		set(WEBP_BUILD_DWEBP OFF CACHE INTERNAL "" FORCE)
		set(WEBP_BUILD_GIF2WEBP OFF CACHE INTERNAL "" FORCE)
		set(WEBP_BUILD_IMG2WEBP OFF CACHE INTERNAL "" FORCE)
		set(WEBP_BUILD_VWEBP OFF CACHE INTERNAL "" FORCE)
		set(WEBP_BUILD_WEBPINFO OFF CACHE INTERNAL "" FORCE)
		set(WEBP_BUILD_LIBWEBPMUX OFF CACHE INTERNAL "" FORCE)
		set(WEBP_BUILD_EXTRAS OFF CACHE INTERNAL "" FORCE)
		
		add_subdirectory(lib/vendor/libwebp)
	endif()
	
	# Build libtiff
	if (${ENABLE_TIFF})
		set(tiff-tools OFF CACHE INTERNAL "" FORCE)
		set(tiff-tests OFF CACHE INTERNAL "" FORCE)
		set(tiff-docs OFF CACHE INTERNAL "" FORCE)
		set(tiff-install OFF CACHE INTERNAL "" FORCE)
	
		add_subdirectory(lib/vendor/libtiff)
		
		target_link_libraries(tiff PRIVATE zlibstatic)
	endif()
else()
	find_package (JPEG REQUIRED)
	
	if (${ENABLE_WEBP} OR ${ENABLE_PNG} OR ${ENABLE_TIFF})
		find_package (ZLIB REQUIRED)
	endif()
	
	if (${ENABLE_WEBP} OR ${ENABLE_PNG})
		find_package (PNG REQUIRED)
	endif()
	
	if (${ENABLE_WEBP})
		find_package (WEBP REQUIRED)
	endif()
	
	if (${ENABLE_TIFF})
		find_package (TIFF REQUIRED)
	endif()
	
endif()

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

# Add library
link_directories(
	${EXTERNAL_DEPS}/libjpeg-turbo
)

# Compile defs
if (${ENABLE_WEBP} OR ${ENABLE_PNG} OR ${ENABLE_TIFF})
	add_compile_definitions(
		DSTREAM_ENABLE_ZIP
	)
endif()

if (${ENABLE_WEBP} OR ${ENABLE_PNG})
	add_compile_definitions(
		DSTREAM_ENABLE_PNG
	)
endif()
if (${ENABLE_TIFF})
	add_compile_definitions(DSTREAM_ENABLE_TIFF)
endif()
if (${ENABLE_WEBP})
	add_compile_definitions(DSTREAM_ENABLE_WEBP)
endif()

add_library(dstream-static STATIC ${DSTREAM_LIB_SRC})


if(MSVC)
	target_link_libraries(dstream-static
		PUBLIC turbojpeg-static
	)
	
	if (${ENABLE_WEBP} OR ${ENABLE_PNG} OR ${ENABLE_TIFF})
		target_link_libraries(dstream-static
			PUBLIC zlibstatic
		)
	endif()
	
	if (${ENABLE_WEBP} OR ${ENABLE_PNG})
		target_link_libraries(dstream-static
			PUBLIC png_static
		)
	endif()
	
	if (${ENABLE_WEBP})
		target_link_libraries(dstream-static PUBLIC webp)
	endif()
	
	if (${ENABLE_TIFF})
		target_link_libraries(dstream-static PUBLIC tiff)
	endif()

# TODO: selectively add include directories, do the same for Unix
	target_include_directories (dstream-static
		PUBLIC lib
	)
	
	if (${ENABLE_WEBP} OR ${ENABLE_PNG} OR ${ENABLE_TIFF})
		target_include_directories(dstream-static
			PUBLIC lib/vendor/zlib
		)
	endif()
	
	if (${ENABLE_WEBP} OR ${ENABLE_PNG})
		target_include_directories(dstream-static
			PUBLIC lib/vendor/libpng
		)
	endif()
	
	if (${ENABLE_TIFF})
		target_include_directories(dstream-static
			PUBLIC lib/vendor/libtiff
		)
	endif()
	
	if (${ENABLE_WEBP})
		target_include_directories(dstream-static
			PUBLIC lib/vendor/libwebp
		)
	endif()
	
	target_include_directories(dstream-static 
		PUBLIC ${CMAKE_BINARY_DIR}/libjpeg-turbo/src/libjpeg-turbo
		PUBLIC ${EXTERNAL_DEPS}/libjpeg-turbo
	)
else()
	target_link_libraries(dstream-static
		PUBLIC ${JPEG_LIBRARIES}
	)
	
	if (${ENABLE_PNG} OR ${ENABLE_WEBP} OR ${ENABLE_TIFF})
		target_link_libraries(dstream-static
			PUBLIC ${ZLIB_LIBRARIES}
		)
	endif()
	
	if (${ENABLE_PNG} OR ${ENABLE_WEBP})
		target_link_libraries(dstream-static
			PUBLIC ${PNG_LIBRARIES}
		)
	endif()
	
	if (${ENABLE_WEBP})
		target_link_libraries(dstream-static
			PUBLIC ${WEBP_LIBRARIES}
		)
	endif()
	
	if (${ENABLE_TIFF})
		target_link_libraries(dstream-static
			PUBLIC ${TIFF_LIBRARIES}
		)
	endif()
	
	
	target_include_directories (dstream-static
		PUBLIC lib
		${JPEG_INCLUDE_DIR}
	)
	
	if (${ENABLE_PNG} OR ${ENABLE_WEBP} OR ${ENABLE_TIFF})
		target_include_directories(dstream-static
			PUBLIC ${ZLIB_INCLUDE_DIR}
		)
	endif()
	
	if (${ENABLE_PNG} OR ${ENABLE_WEBP})
		target_include_directories(dstream-static
			PUBLIC ${PNG_INCLUDE_DIR}
		)
	endif()
	
	if (${ENABLE_WEBP})
		target_include_directories(dstream-static
			PUBLIC ${WEBP_INCLUDE_DIR}
		)
	endif()
	
	if (${ENABLE_WEBP})
		target_include_directories(dstream-static
			PUBLIC ${TIFF_INCLUDE_DIR}
		)
	endif()
endif()

# Add benchmark
if (BUILD_DSTREAM_BENCHMARK)
	set (DSTREAM_BENCHMARK_SRC
		benchmark/DepthmapReader.cpp
		benchmark/ImageReader.cpp
		benchmark/ImageWriter.cpp
		benchmark/Main.cpp
		benchmark/Timer.cpp
		benchmark/JpegEncoder.cpp
		benchmark/JpegDecoder.cpp
		
		benchmark/DepthmapReader.h
		benchmark/ImageWriter.h
		benchmark/ImageReader.h
		benchmark/Timer.h
		
		benchmark/JpegEncoder.h
		benchmark/JpegDecoder.h
		benchmark/stb_image.h
		benchmark/stb_image_write.h
	)
	
	add_executable(dstream-benchmark ${DSTREAM_BENCHMARK_SRC})
	target_include_directories (dstream-benchmark
		PRIVATE benchmark
		PRIVATE lib
	)
	target_link_libraries(dstream-benchmark
		PUBLIC dstream-static
	)
endif()

# Add CMD executable
if (BUILD_DSTREAM_CMD)
	set (DSTREAM_CMD_SRC
		cmd/Main.cpp
		benchmark/DepthmapReader.cpp
		benchmark/ImageReader.cpp
		benchmark/ImageWriter.cpp
		benchmark/JpegDecoder.cpp
		benchmark/JpegEncoder.cpp
		
		benchmark/DepthmapReader.h
		benchmark/ImageWriter.h
		benchmark/ImageReader.h
		benchmark/JpegEncoder.h
		benchmark/JpegDecoder.h
		benchmark/stb_image.h
		benchmark/stb_image_write.h
	)
	
	add_executable(dstream-cmd ${DSTREAM_CMD_SRC})
	target_include_directories (dstream-cmd
		PRIVATE lib
		PRIVATE cmd
		PRIVATE benchmark
	)
	target_link_libraries(dstream-cmd
		PUBLIC dstream-static
	)
endif()

