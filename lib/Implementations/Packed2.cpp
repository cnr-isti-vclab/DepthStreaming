#include "Packed2.h"

namespace DStream
{
	Packed2::Packed2(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistribution) : Coder(quantization, algoBits, channelDistribution) {}

	Color Packed2::EncodeValue(uint16_t val)
	{
		Color ret;
		uint32_t right = m_Quantization - m_AlgoBits;

		ret.x = (val >> right) << (8 - m_AlgoBits);
		ret.y = (val & ((1 << right) - 1)) << (8 - right);
		ret.z = 0;

		return ret;
	}

	uint16_t Packed2::DecodeValue(Color col)
	{
		uint16_t highPart, lowPart;
		uint32_t right = m_Quantization - m_AlgoBits;

		col.x = std::round((float)col.x / (1 << (8 - m_AlgoBits)));
		col.y = std::round((float)col.y / (1 << (8 - right)));

		highPart = col.x << right;
		lowPart = col.y;

		return (highPart + lowPart);
	}
}