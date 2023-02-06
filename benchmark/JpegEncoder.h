#pragma once

#include <cstdlib>
#include <cstdio>
#include <cstdint>

#include <jpeglib.h>

namespace DStream
{
	class JpegEncoder
	{
	public:
		JpegEncoder();
		~JpegEncoder();

		JpegEncoder(const JpegEncoder&) = delete;
		void operator=(const JpegEncoder&) = delete;

		// JCS_GRAYSCALE, JCS_RGB, JCS_YCbCr, or JCS_CMYK.
		void setColorSpace(J_COLOR_SPACE colorSpace, int numComponents);
		void setJpegColorSpace(J_COLOR_SPACE colorSpace);
		J_COLOR_SPACE getColorSpace() const;
		int getNumComponents() const;
		void setQuality(int quality);
		int getQuality() const;
		void setOptimize(bool optimize);
		void setChromaSubsampling(bool subsample);

		bool encode(uint8_t* img, int width, int height, FILE* file);
		bool encode(uint8_t* img, int width, int height, const char* path);
		bool encode(uint8_t* img, int width, int height, uint8_t*& buffer, int& length);

		bool init(int width, int height, uint8_t** buffer, unsigned long* size);
		bool writeRows(uint8_t* rows, int n);
		size_t finish(); //return size

	private:
		bool init(int width, int height);
		bool encode(uint8_t* img, int width, int height);
		static void onError(j_common_ptr cinfo);
		static void onMessage(j_common_ptr cinfo);

		FILE* file = nullptr;
		jpeg_compress_struct info;
		jpeg_error_mgr errMgr;

		J_COLOR_SPACE colorSpace = JCS_RGB;
		J_COLOR_SPACE jpegColorSpace = JCS_YCbCr;
		int numComponents = 3;
		bool optimize = true;
		bool subsample = false;

		int quality = 90;
	};
}
