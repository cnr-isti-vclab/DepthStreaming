#include "Split2.h"
#include <math.h>

namespace DStream
{
	Split2::Split2(uint8_t algoBits, std::vector<uint8_t> channelDistribution)
		: Coder(algoBits, channelDistribution) {}


	Color Split2::EncodeValue(uint16_t val)
	{
		Color ret;
		uint32_t right = std::min(16 - m_AlgoBits, 8);
		val >>= 16 - (right + m_AlgoBits);

		ret.x = val >> right;
		ret.y = (val & ((1 << right) - 1));
		if((ret.x & 0x1) == 1)
			ret.y = (1 << right) - ret.y -1;

		ret.x <<= (8 - m_AlgoBits);
		ret.y <<= (8 - right);
		ret.z = 0;

		std::swap(ret.x, ret.y);
		return ret;
	}

	uint16_t Split2::DecodeValue(Color col)
	{
		std::swap(col.x, col.y);
		uint16_t highPart, lowPart;
		uint32_t right = std::min(16 - m_AlgoBits, 8);

		col.x = std::round((float)col.x / (1 << (8 - m_AlgoBits)));
		col.y = std::round((float)col.y / (1 << (8 - right)));

		if((col.x & 0x1) == 1)
			col.y = (1<<right) - col.y - 1;

		highPart = col.x << right;
		lowPart = col.y;

		return (highPart + lowPart) << (16 - (m_AlgoBits+right));
	}
}
