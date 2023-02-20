#pragma once

#include <string>

namespace DStream
{
	class ImageReader
	{
	public:
		static void ReadJPEG(const std::string& path, uint8_t* dest);
		static void ReadPNG(const std::string& path, uint8_t* dest);
		static void ReadWEBP(const std::string& path, uint8_t* dest, int nElements);

		static void ReadSplitWEBP(const std::string& path, uint8_t* dest, int nElements);
	};
}