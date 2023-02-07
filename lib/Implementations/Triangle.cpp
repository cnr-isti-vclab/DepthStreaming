#include "Triangle.h"

namespace DStream
{
	Triangle::Triangle(uint8_t quantization, bool enlarge, uint8_t algoBits) : Coder(quantization, enlarge, algoBits) {}

	Color Triangle::EncodeValue(uint16_t val)
	{
        const float w = 65536.0f;
        const float p = 512.0f / w;
        Color ret;

        float Ld, Ha, Hb;
        Ld = (val + 0.5) / w;

        float mod = fmod(Ld / (p / 2.0f), 2.0f);
        if (mod <= 1)
            Ha = mod;
        else
            Ha = 2 - mod;

        float mod2 = fmod((Ld - p / 4.0f) / (p / 2.0f), 2.0f);
        if (mod2 <= 1)
            Hb = mod2;
        else
            Hb = 2 - mod2;

        Ld *= 255; Ha *= 255; Hb *= 255;
        ret[0] = Ld; ret[1] = Ha; ret[2] = Hb;

        return ret;
	}

	uint16_t Triangle::DecodeValue(Color col)
	{
        const int w = 65536;
        // Function data
        int np = 512;
        float p = (float)np / w;
        float Ld = col.x / 255.0f;
        float Ha = col.y / 255.0f;
        float Hb = col.z / 255.0f;
        int m = (int)std::floor(4.0 * (Ld / p) - 0.5f) % 4;
        float L0 = Ld - (fmod(Ld - p / 8.0f, p)) + (p / 4.0) * m - p / 8.0;
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

        return (L0 + delta) * w;
	}
}