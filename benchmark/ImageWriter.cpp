#include <ImageWriter.h>
#include <DataStructs/Vec3.h>
#include <JpegEncoder.h>
#ifdef DSTREAM_ENABLE_PNG
    #include <png.h>
#else
    #define STB_IMAGE_WRITE_IMPLEMENTATION
    extern "C"
    {
        #include <stb_image_write.h>
    }
#endif

#ifdef DSTREAM_ENABLE_WEBP
#include <webp/encode.h>
#endif

#include <fstream>
#include <algorithm>

// TODO: add back stbi_image

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
#ifdef DSTREAM_ENABLE_PNG
        ImageWriter::WritePNG(path, (uint8_t*)colorData, width, height);
#else
        WritePNG(path, (uint8_t*)colorData, width, height);
#endif
        delete[] colorData;
    }

    void ImageWriter::WritePNG(const std::string& path, uint8_t* data, uint32_t width, uint32_t height)
    {
#ifdef DSTREAM_ENABLE_PNG
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

        png_destroy_write_struct(&s, &pi);
        
        fclose(fp);
#else
        stbi_write_png(path.c_str(), width, height, 3, data, width * 3);
#endif
    }

#ifdef DSTREAM_ENABLE_WEBP
    void ImageWriter::WriteWEBP(const std::string& path, uint8_t* data, uint32_t width, uint32_t height, uint32_t quality /*= 0*/)
    {
        /*
        WebPConfig config;
        WebPConfigInit(&config);

        int ret = WebPConfigPreset(&config, WebPPreset::WEBP_PRESET_DEFAULT, quality);
        config.filter_strength = 0;
        config.filter_type = 0;
        config.method = 6;
        config.exact = true;
        config.filter_sharpness = 0;
        config.near_lossless = 100;
        config.sns_strength = 0;
        config.use_sharp_yuv = true;
        config.pass = 10;
        config.segments = 4;
        int config_error = WebPValidateConfig(&config);

        WebPPicture pic;
        WebPPictureInit(&pic);
        pic.width = width;
        pic.height = height;
        WebPPictureAlloc(&pic);

        WebPMemoryWriter writer;
        WebPMemoryWriterInit(&writer);
        pic.writer = WebPMemoryWrite;
        pic.custom_ptr = &writer;
        pic.use_argb = false;

        WebPPictureImportRGB(&pic, data, width * 3);
        WebPEncode(&config, &pic);

        // Write green, write red

        std::ofstream outFile;
        outFile.open(path, std::ios::out | std::ios::binary);
        outFile.write((const char*)writer.mem, writer.size);
        outFile.close();

        WebPPictureFree(&pic);*/

        uint8_t* buf;
        size_t wrote;

        if (quality == 0)
            wrote = WebPEncodeLosslessRGB(data, width, height, width * 3, &buf);
        else
            wrote = WebPEncodeRGB(data, width, height, width * 3, quality, &buf);

        std::ofstream outFile;
        outFile.open(path, std::ios::out | std::ios::binary);
        outFile.write((const char*)buf, wrote);
        outFile.close();
    }


    void ImageWriter::WriteSplitWEBP(const std::string& path, uint8_t* data, uint32_t width, uint32_t height, uint32_t quality /*= 0*/)
    {
        uint8_t* redData = new uint8_t[width * height * 3];
        uint8_t* greenData = new uint8_t[width * height * 3];

        for (uint32_t i = 0; i < width * height * 3; i += 3)
        {
            redData[i] = data[i];
            redData[i + 1] = data[i];
            redData[i + 2] = data[i];

            greenData[i] = data[i + 1];
            greenData[i + 1] = data[i + 1];
            greenData[i + 2] = data[i + 1];
        }

        WebPConfig config;
        WebPConfigInit(&config);

        int ret = WebPConfigPreset(&config, WebPPreset::WEBP_PRESET_DEFAULT, quality);
        config.filter_strength = 0;
        config.filter_type = 0;
        config.method = 4;
        config.exact = true;
        config.filter_sharpness = 0;
        config.near_lossless = 100;
        config.sns_strength = 0;
        config.use_sharp_yuv = true;
        int config_error = WebPValidateConfig(&config);

        WebPPicture redPic;
        WebPPicture greenPic;

        WebPPictureInit(&redPic);
        WebPPictureInit(&greenPic);

        redPic.width = width;
        redPic.height = height;
        greenPic.width = width;
        greenPic.height = height;

        WebPMemoryWriter redWriter;
        WebPMemoryWriterInit(&redWriter);
        redPic.writer = WebPMemoryWrite;
        redPic.custom_ptr = &redWriter;
        redPic.use_argb = false;

        WebPMemoryWriter greenWriter;
        WebPMemoryWriterInit(&greenWriter);
        greenPic.writer = WebPMemoryWrite;
        greenPic.custom_ptr = &greenWriter;
        greenPic.use_argb = false;

        // Encode and write red
        WebPPictureImportRGB(&redPic, redData, width * 3);
        WebPEncode(&config, &redPic);

        std::ofstream outFile;
        outFile.open(path + ".red.webp", std::ios::out | std::ios::binary);
        outFile.write((const char*)redWriter.mem, redWriter.size);
        outFile.close();
        WebPPictureFree(&redPic);

        // Encode and write green
        WebPPictureImportRGB(&greenPic, greenData, width * 3);
        WebPEncode(&config, &greenPic);

        std::ofstream greenFile;
        greenFile.open(path + ".green.webp", std::ios::out | std::ios::binary);
        greenFile.write((const char*)greenWriter.mem, greenWriter.size);
        greenFile.close();
        WebPPictureFree(&greenPic);

        delete[] redData;
        delete[] greenData;
    }
#endif
}