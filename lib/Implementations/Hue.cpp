#include "Hue.h"
#include <math.h>

namespace DStream
{
	Hue::Hue(uint8_t quantization, uint8_t algoBits) : Coder(quantization, algoBits) {}

	Color Hue::EncodeValue(uint16_t val)
	{
        Color ret;
        uint16_t d = std::round(((float)val / ((1 << m_Quantization) - 1)) * 1529.0f);

        if ((d <= 255) || (1275 < d && d <= 1529))
            ret.x = 255;
        else if (255 < d && d <= 510)
            ret.x = 255 - d;
        else if (510 < d && d <= 1020)
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

        if (d < 510)
            ret.z = 0;
        else if (510 < d && d <= 765)
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

        float q = std::round(((float)ret / 1529.0f) * (1 << m_Quantization));
        return q;
	}
}
