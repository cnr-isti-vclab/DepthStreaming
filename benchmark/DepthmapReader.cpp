#include <DepthmapReader.h>

#include <libtiff/tiff.h>
#include <libtiff/tiffio.h>

#include <iostream>
#include <fstream>
#include <filesystem>

namespace DStream
{
    DepthmapReader::DepthmapReader(const std::string& path, DepthmapFormat format, DepthmapData& dmData, bool quantize /* = true*/)
    {
        switch (format)
        {
        case DepthmapFormat::ASC:
            ParseASC(path, dmData, quantize);
            break;
        case DepthmapFormat::TIF:
            ParseTIFF(path, dmData, quantize);
            break;
        case DepthmapFormat::PGM:
            ParsePGM(path, dmData, quantize);
            break;
        default:
            std::cerr << "Unsupported depthmap input format" << std::endl;
            break;
        }
    }

    DepthmapReader::~DepthmapReader()
    {
        delete[] m_Data;
    }

    void DepthmapReader::ParseASC(const std::string& path, DepthmapData& dmData, bool quantize)
    {
        if (!std::filesystem::exists(path))
        {
            std::cerr << "Input file " << path << " does not exist" << std::endl;
            return;
        }

        // Load asc data
        FILE* fp = fopen(path.c_str(), "rb");

        if (!fp)
        {
            std::cerr << "Could not open: " << path << std::endl;
            return;
        }

        float nodata;
        fscanf(fp, "ncols %d\n", &dmData.Width);
        fscanf(fp, "nrows %d\n", &dmData.Height);
        fscanf(fp, "xllcenter %f\n", &dmData.CenterX);
        fscanf(fp, "yllcenter %f\n", &dmData.CenterY);
        fscanf(fp, "cellsize %f\n", &dmData.CellSize);
        fscanf(fp, "nodata_value %f\n", &nodata);

        // Load depth data
        m_Data = new uint16_t[dmData.Width * dmData.Height];
        float* tmp = new float[dmData.Width * dmData.Height];
        float min = 1e20;
        float max = -1e20;

        for (uint32_t i = 0; i < dmData.Width * dmData.Height; i++)
        {
            float h;
            fscanf(fp, "%f", &h);
            min = std::min(min, h);
            max = std::max(max, h);
            tmp[i] = h;
        }

        // Quantize
        if (quantize)
            for (uint32_t i = 0; i < dmData.Width * dmData.Height; i++)
                m_Data[i] = ((tmp[i] - min) / (float)(max - min)) * 65535.0f;

        dmData.Valid = true;
        fclose(fp);
        delete[] tmp;
    }

    void DepthmapReader::ParseTIFF(const std::string& path, DepthmapData& dmData, bool quantize)
    {
        TIFF* inFile = TIFFOpen(path.c_str(), "r");
        int width, height, stripSize, nStrips;
        int16_t min = 32767, max = -32768;

        TIFFGetField(inFile, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(inFile, TIFFTAG_IMAGELENGTH, &height);
        stripSize = TIFFStripSize(inFile);
        nStrips = TIFFNumberOfStrips(inFile);

        dmData.Width = width;
        dmData.Height = height;
        int16_t* tmpBuffer = new int16_t[width * height];
        m_Data = new uint16_t[width * height];
        
        tdata_t buf = _TIFFmalloc(stripSize);
        tstrip_t strip;

        uint32_t read = 0;

        for (strip = 0; strip < nStrips; strip++)
        {
            TIFFReadEncodedStrip(inFile, strip, buf, stripSize);
            memcpy(tmpBuffer + read, buf, stripSize);
            
            read += stripSize / sizeof(int16_t);
        }

        if (quantize)
        {
            for (uint32_t i = 0; i < width * height; i++)
            {
                min = std::min(min, tmpBuffer[i]);
                max = std::max(max, tmpBuffer[i]);
            }

            for (uint32_t i=0; i<width * height; i++)
                m_Data[i] = ((float)(tmpBuffer[i] - min) / (max - min)) * 65535.0f;
        }

        _TIFFfree(buf);
        TIFFClose(inFile);

        delete[] tmpBuffer;
        dmData.Valid = true;
    }

    void DepthmapReader::ParsePGM(const std::string& path, DepthmapData& dmData, bool quantize)
    {
        int width, height;
        std::string dummy;
        std::ifstream file(path, std::ios::in | std::ios::binary);
        uint16_t min = 65535, max = 0;
        
        std::getline(file, dummy);
        std::getline(file, dummy);

        int spaceIdx = dummy.find_first_of(" ");
        width = atoi(dummy.substr(0, spaceIdx).c_str());
        height = atoi(dummy.substr(spaceIdx + 1, dummy.length() - spaceIdx).c_str());

        std::getline(file, dummy);

        dmData.Width = width;
        dmData.Height = height;

        m_Data = new uint16_t[dmData.Width * dmData.Height];
        file.read(reinterpret_cast<char*>(m_Data), width * height * sizeof(uint16_t));

        // Invert endianess
        for (uint32_t i = 0; i < width * height; i++)
            m_Data[i] = (m_Data[i] >> 8) | (m_Data[i] << 8);

        if (quantize)
        {
            for (uint32_t i = 0; i < width * height; i++)
            {
                min = std::min(min, m_Data[i]);
                max = std::max(max, m_Data[i]);
            }

            for (uint32_t i = 0; i < width * height; i++)
                m_Data[i] = ((float)(m_Data[i] - min) / (max - min)) * 65535.0f;
        }

        dmData.Valid = true;
    }
}