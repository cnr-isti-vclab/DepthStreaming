#include "Triangle.h"
#include <math.h>

namespace DStream
{
	Triangle::Triangle(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistribution) : Coder(quantization, algoBits, channelDistribution) {}

	Color Triangle::EncodeValue(uint16_t val)
	{
        Color ret;
        val <<= (16 - m_Quantization);
        uint32_t p = (1 << 9);

        uint8_t Ld, Ha, Hb;
        Ld = val >> 8;

        float mod = fmod((float)val / (p >> 1), 2.0f);
        if (mod <= 1) Ha = mod * 256;
        else Ha = (2 - mod) *256;

        float mod2 = fmod(((float)((int)val - (p >> 2)) / (p >> 1)), 2.0f);
        if (mod2 <= 1) Hb = mod2 * 256;
        else Hb = (2 - mod2) * 256;

        ret[0] = Ld; ret[1] = Ha; ret[2] = Hb;
        return ret;
	}

	uint16_t Triangle::DecodeValue(Color col)
	{
        const int w = 1 << 16;
        const int maxVal = 1 << m_Quantization;
        // Function data
        int np = 512;
        float p = (float)np / w;
        float Ld = col.x / 255.0f;
        float Ha = col.y / 255.0f;
        float Hb = col.z / 255.0f;
        int m = (int)std::floor(4.0 * (Ld / p) - (Ld > 0 ? 0.5f : 0.0f)) % 4;
        float L0 = (Ld - (fmod(Ld - p / 8.0f, p)) + (p / 4.0) * m - p / 8.0);
        float delta = 0;

        switch (m) {
        case 0:
            delta = (p / 2.0f) * Ha;
            break;
        case 1:
            delta = (p / 2.0f) * Hb;
            break;
        case 2:
            delta = (p / 2.0f) * (1.0f - Ha);
            break;
        case 3:
            delta = (p / 2.0f) * (1.0f - Hb);
            break;
        }

        return (L0 + delta) * maxVal;
	}
}
