#include <ImageWriter.h>
#include <DataStructs/Vec3.h>
#include <JpegEncoder.h>
#include <png.h>

#include <StbImport.cpp>
#include <stb_image.h>
#include <stb_image_write.h>

#include <fstream>

namespace DStream
{
    void ImageWriter::WriteEncoded(const std::string& path, uint8_t* data, uint32_t width, uint32_t height, ImageFormat format, uint32_t quality /* = 100*/)
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

    void ImageWriter::WriteError(const std::string& path, uint8_t* data, uint32_t width, uint32_t height)
    {
        WritePNG(path, data, width, height);
    }

    void ImageWriter::WritePNG(const std::string& path, uint8_t* data, uint32_t width, uint32_t height)
    {
        png_image image;
        memset(&image, 0, sizeof image);
        image.version = PNG_IMAGE_VERSION;
        image.width = width;
        image.height = height;

        png_image_write_to_file(&image, path.c_str(), 0, data, 0, NULL);
    }
}