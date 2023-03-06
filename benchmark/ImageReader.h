#pragma once

#include <string>

namespace DStream
{
	class ImageReader
	{
	public:
		static void Read(const std::string& path, uint8_t* dest, uint32_t dataSize);

		static void ReadJPEG(const std::string& path, uint8_t* dest);
		static void ReadPNG(const std::string& path, uint8_t* dest);

#ifdef DSTREAM_ENABLE_WEBP
		static void ReadWEBP(const std::string& path, uint8_t* dest, int nElements);
		static void ReadSplitWEBP(const std::string& path, uint8_t* dest, int nElements);
#endif

		static void GetImageSize(const std::string& path, int* width, int* height, int* comp, const std::string& extesion = "");
	};
}