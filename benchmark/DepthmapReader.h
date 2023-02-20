#pragma once

#include <cstdint>
#include <string>
#include <memory>

namespace DStream
{
    enum DepthmapFormat { NONE = 0, ASC, TIF, DEM, XYZ };

    struct DepthmapData
    {
        bool Valid = false;

        uint32_t Width;
        uint32_t Height;

        float CenterX;
        float CenterY;
        float CellSize;

        DepthmapData() = default;
        DepthmapData(const DepthmapData& data) = default;
    };

	class DepthmapReader
	{
    public:
        DepthmapReader() = default;
        DepthmapReader(const std::string& path, DepthmapFormat format, DepthmapData& dmData, bool quantize = true);
        ~DepthmapReader();
        inline uint16_t* GetData() { return m_Data; }

    private:
        void ParseASC(const std::string& path, DepthmapData& dmData, bool quantize);
        void ParseTIFF(const std::string& path, DepthmapData& dmData, bool quantize);
        void ParseDEM(const std::string& path, DepthmapData& dmData, bool quantize);
        void ParseXYZ(const std::string& path, DepthmapData& dmData, bool quantize);

    private:
        uint16_t* m_Data;
    };
}