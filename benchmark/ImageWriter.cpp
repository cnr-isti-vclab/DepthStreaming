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
    /*

    SaveJPEG(QString(m_OutputPath.c_str()), data, quality);
    // Save json data
    QFile info("info.json");
    if(!info.open(QFile::WriteOnly))
        throw "Failed writing info.json";

    QJsonDocument doc;
    QJsonObject obj;

    obj.insert("width", QJsonValue::fromVariant(m_Width));
    obj.insert("height", QJsonValue::fromVariant(m_Height));
    obj.insert("type", QJsonValue::fromVariant("dem"));
    obj.insert("encoding", QJsonValue::fromVariant(EncodingModeToStr(props.Mode)));
    // TODO: the result of an Encode function is a structure containing a set of images and additional
    // properties to be saved in form of a QJsonObject
    //obj.insert("p", QJsonValue::fromVariant(p));
    obj.insert("cellsize", QJsonValue::fromVariant(m_CellSize));
    obj.insert("min", QJsonValue::fromVariant(m_Min));
    obj.insert("max", QJsonValue::fromVariant(m_Max));

    doc.setObject(obj);
    QTextStream stream(&info);
    stream << doc.toJson();

    }
    void Writer::SaveJPEG(const QString& path, const uint8_t* data, uint32_t quality)
    {
        JpegEncoder encoder;
        uint8_t* retBuffer = new uint8_t[sourceImage.width() * sourceImage.height() * 3];
        unsigned long retSize;
        QImage src = sourceImage.convertToFormat(QImage::Format_RGB888);

        encoder.setJpegColorSpace(J_COLOR_SPACE::JCS_RGB);
        encoder.setQuality(quality);

        encoder.init(src.width(), src.height(), &retBuffer, &retSize);
        encoder.writeRows(src.bits(), src.height());
        encoder.finish();

        QFile out(path);
        out.open(QIODevice::WriteOnly);
        out.write((const char*)retBuffer, retSize);
        out.close();

        //delete[] retBuffer;
    }
 */
}