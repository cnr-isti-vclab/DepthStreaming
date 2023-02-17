#pragma once

#include <cstdint>
#include <string>

namespace DStream
{
	class ImageWriter
	{
	public:
		static void WriteDecoded(const std::string& path, uint16_t* data, uint32_t width, uint32_t height);

		static void WriteJPEG(const std::string& path, uint8_t* data, uint32_t width, uint32_t height, uint32_t quality = 100);
		static void WritePNG(const std::string& path, uint8_t* data, uint32_t width, uint32_t height);
		static void WriteWEBP(const std::string& path, uint8_t* data, uint32_t width, uint32_t height, uint32_t quality = 0);

	};
}