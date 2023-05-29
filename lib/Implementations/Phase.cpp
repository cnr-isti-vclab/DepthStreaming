#include "Phase.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace DStream
{
	Phase::Phase(uint8_t algoBits, std::vector<uint8_t> channelDistribution)
		: Coder(algoBits, channelDistribution) {}

	Color Phase::EncodeValue(uint16_t val)
	{
		val <<= (16 - 3 * m_AlgoBits);
		Color ret;
		const float P = 1 << 14;
		const float w = 1 << 16;

		ret[0] = 255 * (0.5f + 0.5f * std::cos(M_PI * 2.0f * (val / P)));
		ret[1] = 255 * (val / w);
		ret[2] = 0;

		return ret;
	}

	uint16_t Phase::DecodeValue(Color col)
	{
		const float w = 1 << 16;
		const float P = w / 4;
		const float beta = P / 2.0f;
		float gamma, phi, PHI, K, Z;
		float i1 = col.x / 255.0f, i2 = col.y / 255.0f;

		phi = std::fabs(std::acos(2.0f * i1 - 1.0f));
		gamma = std::floor((i2 * w) / beta);

		if (((int)gamma) % 2)
			phi *= -1;

		K = std::round((i2 * w) / P);
		PHI = phi + 2 * M_PI * K;

		Z = PHI * (P / (M_PI * 2.0f));
		return std::min(std::max(0, static_cast<int>(Z)), 1 << 16) >> (16-m_AlgoBits*3);
	}
}