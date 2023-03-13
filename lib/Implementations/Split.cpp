#include "Split.h"
#include <math.h>

namespace DStream
{
	Split::Split(uint8_t quantization, uint8_t algoBits) : Coder(quantization, algoBits) {}


	Color Split::EncodeValue(uint16_t val)
	{
		Color ret;
		uint32_t right = m_Quantization - m_AlgoBits;

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

	uint16_t Split::DecodeValue(Color col)
	{
		std::swap(col.x, col.y);
		uint16_t highPart, lowPart;
		uint32_t right = m_Quantization - m_AlgoBits;

		col.x >>= (8 - m_AlgoBits);
		col.y >>= (8 - right);

		if((col.x & 0x1) == 1)
			col.y = (1<<right) - col.y - 1;

		highPart = col.x << right;
		lowPart = col.y;

		return (highPart + lowPart);
	}
}
