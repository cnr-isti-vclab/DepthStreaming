#include <ImageReader.h>
#include <JpegDecoder.h>

#ifdef DSTREAM_ENABLE_PNG
	#include <png.h>
#else
	#define STB_IMAGE_IMPLEMENTATION
	extern "C"
	{
		#include <stb_image.h>
	}
#endif

#ifdef DSTREAM_ENABLE_WEBP
	#include <webp/decode.h>
#endif

#include <fstream>
#include <sstream>

namespace DStream
{
	void ImageReader::GetImageSize(const std::string& path, int* width, int* height, int* comp, const std::string& ext)
	{
		std::string extension;
		if (ext == "")
		{
			uint32_t extStart = path.find_last_of(".");
			extension = path.substr(extStart, path.length() - extStart);
		}
		else
			extension = ext;

		for (uint32_t i = 0; i < extension.length(); i++)
			extension[i] = tolower(extension[i]);

		if (extension == ".jpg")
		{
			JpegDecoder jd;
			jd.init(path.c_str(), *width, *height);
			*comp = 3;
		}
		else if (extension == ".png")
		{
#ifdef DSTREAM_ENABLE_PNG
			FILE* fp = fopen(path.c_str(), "rb");
			png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			png_infop info_ptr = png_create_info_struct(png_ptr);

			png_init_io(png_ptr, fp);
			png_read_info(png_ptr, info_ptr);
			*width = png_get_image_width(png_ptr, info_ptr);
			*height = png_get_image_height(png_ptr, info_ptr);

			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(fp);
#else
			stbi_info(path.c_str(), width, height, comp);
#endif
		}
#ifdef DSTREAM_ENABLE_WEBP
		else if (extension == ".webp")
		{
			FILE* fp = fopen(path.c_str(), "rb");
			int maxWebpHeaderSize = 30;

			uint8_t* headerData = new uint8_t[maxWebpHeaderSize];
			uint32_t read = fread(headerData, 1, maxWebpHeaderSize, fp);

			WebPGetInfo(headerData, read, width, height);

			fclose(fp);
			delete[] headerData;
		}
		else if (extension == ".splitwebp")
		{
			FILE* fp = fopen((path.substr(0, path.find_last_of(".")) + ".green.splitwebp").c_str(), "rb");
			int maxWebpHeaderSize = 30;

			uint8_t* headerData = new uint8_t[maxWebpHeaderSize];
			uint32_t read = fread(headerData, 1, maxWebpHeaderSize, fp);

			WebPGetInfo(headerData, read, width, height);

			fclose(fp);
			delete[] headerData;
		}
#endif
	}

	void ImageReader::Read(const std::string& path, uint8_t* dest, uint32_t dataSize)
	{
		uint32_t extStart = path.find_last_of(".");
		std::string extension = path.substr(extStart, path.length() - extStart);
		for (uint32_t i = 0; i < extension.length(); i++)
			extension[i] = tolower(extension[i]);

		if (extension == ".jpg")
			ReadJPEG(path, dest);
		else if (extension == ".png")
			ReadPNG(path, dest);
#ifdef DSTREAM_ENABLE_WEBP
		else if (extension == ".webp")
			ReadWEBP(path, dest, dataSize);
		else if (extension == ".splitwebp")
			ReadSplitWEBP(path, dest, dataSize);
#endif
	}

	void ImageReader::ReadJPEG(const std::string& path, uint8_t* dest)
	{
		// JPEG ENCODING / DECODING
		JpegDecoder decoder; int w, h;
		decoder.setJpegColorSpace(J_COLOR_SPACE::JCS_RGB);
		decoder.decodeNonAlloc(path.c_str(), dest, w, h);
	}

	void ImageReader::ReadPNG(const std::string& path, uint8_t* dest)
	{
#ifdef DSTREAM_ENABLE_PNG
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
#else
		int w, h, comp;
		uint8_t* data = stbi_load(path.c_str(), &w, &h, &comp, 3);
		memcpy(dest, data, w * h * 3);
		stbi_image_free(data);
#endif
	}

#ifdef DSTREAM_ENABLE_WEBP
	void ImageReader::ReadWEBP(const std::string& path, uint8_t* dest, int nElements)
	{
		FILE* fp = fopen(path.c_str(), "rb");
		uint8_t* fileData = new uint8_t[nElements];
		size_t read = fread(fileData, sizeof(char), nElements, fp);
		int w, h;

		WebPGetInfo(fileData, read, &w, &h);
		WebPDecodeRGBInto(fileData, read, dest, nElements, w * 3);

		delete[] fileData;
	}

	void ImageReader::ReadSplitWEBP(const std::string& path, uint8_t* dest, int nElements)
	{
		// Remove extension
		std::string parentPath = path.substr(0, path.find_last_of("."));
		// Load green and red files
		FILE* redFp = fopen((parentPath + ".red.splitwebp").c_str(), "rb");
		FILE* greenFp = fopen((parentPath + ".green.splitwebp").c_str(), "rb");

		uint8_t* redData = new uint8_t[nElements];
		uint8_t* greenData = new uint8_t[nElements];
		uint8_t* redDest = new uint8_t[nElements];
		uint8_t* greenDest = new uint8_t[nElements];

		size_t redRead = fread(redData, sizeof(char), nElements, redFp);
		size_t greenRead = fread(greenData, sizeof(char), nElements, greenFp);
		int w, h;

		WebPGetInfo(redData, redRead, &w, &h);
		WebPDecodeRGBInto(redData, redRead, redDest, nElements, w * 3);

		WebPGetInfo(greenData, greenRead, &w, &h);
		WebPDecodeRGBInto(greenData, greenRead, greenDest, nElements, w * 3);

		for (uint32_t i = 0; i < nElements; i += 3)
		{
			dest[i] = redDest[i];
			dest[i+1] = greenDest[i];
			dest[i+2] = 0;
		}

		fclose(redFp);
		fclose(greenFp);

		delete[] redData;
		delete[] greenData;
		delete[] redDest;
		delete[] greenDest;
	}
#endif
}