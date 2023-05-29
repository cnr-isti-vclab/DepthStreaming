#include "Hue.h"
#include <math.h>

namespace DStream
{
    Hue::Hue(uint8_t algoBits, std::vector<uint8_t> channelDistribution) : Coder(algoBits, channelDistribution) {}

    Color Hue::EncodeValue(uint16_t val)
    {
        Color ret;
        uint16_t div = 65535;
        uint16_t d = std::round(((float)val / div) * 1529.0f);

        if ((0 <= d && d <= 255) || (1275 < d && d <= 1529))
            ret.x = 255;
        else if (255 <= d && d <= 511)
            ret.x = 255 - d;
        else if (511 < d && d <= 1020)
            ret.x = 0;
        else if (1020 < d && d <= 1275)
            ret.x = d - 1020;

        if (d <= 255)
            ret.y = d;
        else if (255 < d && d <= 765)
            ret.y = 255;
        else if (765 < d && d <= 1020)
            ret.y = 255 - (d - 765);
        else
            ret.y = 0;

        if (d <= 511)
            ret.z = 0;
        else if (511 < d && d <= 765)
            ret.z = d;
        else if (765 < d && d <= 1275)
            ret.z = 255;
        else if (1275 < d && d <= 1529)
            ret.z = 1275 - d;

        return ret;
    }

    uint16_t Hue::DecodeValue(Color col)
    {
        uint8_t r = col.x, g = col.y, b = col.z;
        uint16_t ret = 0;
        uint16_t mul = 65535;

        if (b + g + r < 255)
            ret = 0;
        else if (r >= g && r >= b)
        {
            if (g >= b)
                ret = g - b;
            else
                ret = (g - b) + 1529;
        }
        else if (g >= r && g >= b)
            ret = b - r + 510;
        else if (b >= g && b >= r)
            ret = r - g + 1020;

        float q = std::round(((float)ret / 1529.0f) * mul);
        ret = (uint16_t)q;
        return ret;
    }
}
