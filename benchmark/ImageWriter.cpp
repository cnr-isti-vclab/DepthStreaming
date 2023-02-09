#include <ImageWriter.h>
#include <DataStructs/Vec3.h>
#include <JpegEncoder.h>

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
        stbi_write_png(path.c_str(), width, height, 3, (void*)colorData, 0);
        delete[] colorData;
    }

    void ImageWriter::WriteError(const std::string& path, uint8_t* data, uint32_t width, uint32_t height)
    {
        stbi_write_png(path.c_str(), width, height, 3, (void*)data, 0);
    }
}