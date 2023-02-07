#include <DepthmapReader.h>

#include <iostream>
#include <filesystem>

namespace DStream
{
    DepthmapReader::DepthmapReader(const std::string& path, DepthmapFormat format, DepthmapData& dmData)
    {
        switch (format)
        {
        case DepthmapFormat::ASC:
            ParseASC(path, dmData);
            break;
        default:
            std::cout << "Unsupported depthmap input format" << std::endl;
            break;
        }
    }

    DepthmapReader::~DepthmapReader()
    {
        delete[] m_Data;
    }

    void DepthmapReader::ParseASC(const std::string& path, DepthmapData& dmData)
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
        for (uint32_t i = 0; i < dmData.Width * dmData.Height; i++)
            m_Data[i] = ((tmp[i] - min) / (float)(max - min)) * 65535.0f;

        dmData.Valid = true;
        fclose(fp);
        delete[] tmp;
    }
}