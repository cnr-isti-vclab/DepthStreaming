#include <ImageReader.h>
#include <JpegDecoder.h>

#include <png.h>
#include <webp/decode.h>

#include <fstream>
#include <sstream>

namespace DStream
{
	void ImageReader::ReadJPEG(const std::string& path, uint8_t* dest)
	{
		// JPEG ENCODING / DECODING
		JpegDecoder decoder; int w, h;
		decoder.setJpegColorSpace(J_COLOR_SPACE::JCS_RGB);
		decoder.decodeNonAlloc(path.c_str(), dest, w, h);
	}

	void ImageReader::ReadPNG(const std::string& path, uint8_t* dest)
	{
		FILE* fp = fopen(path.c_str(), "rb");
		png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		png_infop info_ptr = png_create_info_struct(png_ptr);
		
		png_init_io(png_ptr, fp);
		png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
		
		uint32_t width = png_get_image_width(png_ptr, info_ptr);
		uint32_t height = png_get_image_height(png_ptr, info_ptr);
		png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

		for (uint32_t i = 0; i < height; i++)
			memcpy(dest + i * width * 3, row_pointers[i], width * 3);

		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
	}

	void ImageReader::ReadWEBP(const std::string& path, uint8_t* dest, int nElements)
	{
		FILE* fp = fopen(path.c_str(), "rb");
		size_t read = fread(dest, sizeof(char), nElements, fp);
		int w, h;
		uint8_t* ret = WebPDecodeRGB(dest, nElements, &w, &h);
		memcpy(dest, ret, nElements);
		WebPFree(ret);
	}
}