#include <ImageWriter.h>
#include <DataStructs/Vec3.h>
#include <JpegEncoder.h>
#include <png.h>
#include <fstream>
#include <webp/encode.h>

namespace DStream
{
    void ImageWriter::WriteJPEG(const std::string& path, uint8_t* data, uint32_t width, uint32_t height, uint32_t quality /* = 100*/)
    {
        uint8_t* encodedData = new uint8_t[width * height * 3];
        unsigned long retSize;
        JpegEncoder encoder;

        encoder.setJpegColorSpace(J_COLOR_SPACE::JCS_RGB);
        encoder.setQuality(quality);

        encoder.init(width, height, &encodedData, &retSize);
        encoder.writeRows(data, height);
        encoder.finish();

        std::ofstream outFile;
        outFile.open(path, std::ios::out | std::ios::binary);
        outFile.write((const char*)encodedData, retSize);
        outFile.close();

        delete[] encodedData;
    }

    void ImageWriter::WriteDecoded(const std::string& path, uint16_t* data, uint32_t width, uint32_t height)
    {
        Color* colorData = new Color[width * height];
        for (uint32_t i = 0; i < width * height; i++)
        {
            uint8_t quantizedVal = data[i] >> 8;
            colorData[i].x = quantizedVal;
            colorData[i].y = quantizedVal;
            colorData[i].z = quantizedVal;
        }
        ImageWriter::WritePNG(path, (uint8_t*)colorData, width, height);
        delete[] colorData;
    }

    void ImageWriter::WritePNG(const std::string& path, uint8_t* data, uint32_t width, uint32_t height)
    {
        FILE* fp = fopen(path.c_str(), "wb");

        png_image image;
        png_bytep* rows = (png_bytep*)malloc(sizeof(png_bytep) * height);
        png_structp s = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        png_infop pi = png_create_info_struct(s);
        png_byte color_type = PNG_COLOR_TYPE_RGB;

        for (int i = 0; i < height; ++i)
            rows[i] = data + i * 3 * width;
        
        png_init_io(s, fp);
        png_set_IHDR(s, pi, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        png_set_filter(s, 0, PNG_FILTER_NONE);

        png_write_info(s, pi);
        png_write_image(s, rows);
        png_write_end(s, NULL);
        
        fclose(fp);
    }

    void ImageWriter::WriteWEBP(const std::string& path, uint8_t* data, uint32_t width, uint32_t height)
    {
        uint8_t* buf;
        size_t wrote = WebPEncodeLosslessRGB(data, width, height, width * 3, &buf);
        std::ofstream outFile;
        outFile.open(path, std::ios::out | std::ios::binary);
        outFile.write((const char*)buf, wrote);
        outFile.close();
    }

}