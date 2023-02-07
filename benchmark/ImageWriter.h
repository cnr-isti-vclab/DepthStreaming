#pragma once

#include <cstdint>
#include <string>

namespace DStream
{
	enum ImageFormat {PNG = 0, JPG};

	class ImageWriter
	{
	public:
		static void WriteEncoded(const std::string& path, uint8_t* data, uint32_t width, uint32_t height, ImageFormat format, uint32_t quality = 100);
		static void WriteDecoded(const std::string& path, uint16_t* data, uint32_t width, uint32_t height);
	};
}