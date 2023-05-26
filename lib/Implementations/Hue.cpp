#include "Hue.h"
#include <math.h>

namespace DStream
{
    Hue::Hue(uint8_t algoBits, std::vector<uint8_t> channelDistribution) : Coder(algoBits, channelDistribution) {}

    Color Hue::EncodeValue(uint16_t val)
    {
        Color ret;
        uint16_t div = (1 << (m_AlgoBits * 3))-1;
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
        // When ret.z < 255 / ((1 << m_AlgoBits) - 1), ret.z gets rounded to 0. This makes the hue turn to red from blue,
        // causing huge errors. We can avoid that by ensuring there's a minimum of 1 in ret.z and allowing for a higher error, in
        // favor of interpolation and enlargement. Normally, the algorithm would work in a [0, 255] range so it wouldn't need neither this
        // workaround nor the following mapping to [0, (1 << (m_AlgoBits) - 1)]
        else if (1275 < d && d <= 1529)
            ret.z = std::max<uint8_t>(std::ceil((255.0f / (float)((1 << m_AlgoBits) - 1))/2), 1275 - d);

        for (uint32_t i = 0; i < 3; i++)
            ret[i] = std::round(((float)ret[i] / 255.0f) * ((1 << m_AlgoBits)-1));

        return ret;
    }

    uint16_t Hue::DecodeValue(Color col)
    {
        Color col1 = col;
        for (uint32_t i = 0; i < 3; i++)
            col1[i] = std::round(((float)col1[i] / ((1 << m_AlgoBits)-1)) * 255.0f);

        uint8_t r = col1.x, g = col1.y, b = col1.z;
        uint16_t ret = 0;
        uint16_t mul = 1 << (m_AlgoBits * 3);

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
