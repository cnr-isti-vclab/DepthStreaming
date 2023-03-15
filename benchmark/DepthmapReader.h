#pragma once

#include <cstdint>
#include <string>
#include <memory>

// TODO: quantize in the generic function, parse in the specific ones

namespace DStream
{
    enum DepthmapFormat { NONE = 0, ASC, 
#ifdef DSTREAM_ENABLE_TIFF 
        TIF,
#endif 
        DEM, XYZ, PGM };

    struct DepthmapData
    {
        bool Valid = false;

        uint32_t Width = 0;
        uint32_t Height = 0;

        float CenterX = 0;
        float CenterY = 0;
        float CellSize = 0;

        float MinDepth = std::numeric_limits<float>().max();
        float MaxDepth = std::numeric_limits<float>().min();

        DepthmapData() = default;
        DepthmapData(const DepthmapData& data) = default;
    };

	class DepthmapReader
	{
    public:
        DepthmapReader() = default;
        DepthmapReader(const std::string& path, DepthmapData& dmData);
        DepthmapReader(const std::string& path, DepthmapFormat format, DepthmapData& dmData);
        ~DepthmapReader();
        inline float* GetRawData() { return m_Data; }

    private:
        void ParseASC(const std::string& path, DepthmapData& dmData);
#ifdef DSTREAM_ENABLE_TIFF
        void ParseTIFF(const std::string& path, DepthmapData& dmData);
#endif
        void ParsePGM(const std::string& path, DepthmapData& dmData);
        void ParseDEM(const std::string& path, DepthmapData& dmData);
        void ParseXYZ(const std::string& path, DepthmapData& dmData);

    private:
        float* m_Data;
    };
}