#include "Split.h"

namespace DStream
{
	Split::Split(uint8_t quantization, uint8_t algoBits) : Coder(quantization, algoBits) {}


	Color Split::EncodeValue(uint16_t val)
	{
		Color ret;
		uint32_t right = m_Quantization - m_AlgoBits;

		val >>= (16 - m_Quantization);

		ret.x = (val >> right);
		ret.y = (val & ((1 << right) - 1));
		if(ret.x & 0x1 == 1)
			ret.y = (1 << right) - ret.y -1;

		ret.x <<= (8 - m_AlgoBits);
		ret.y <<= (8 - right);
		ret.z = 0;

		return ret;
	}

	uint16_t Split::DecodeValue(Color col)
	{
		uint16_t highPart, lowPart;
		uint32_t right = m_Quantization - m_AlgoBits;

		col.x >>= (8 - m_AlgoBits);
		col.y >>= (8 - right);

		if(col.x & 0x1 == 1)
			col.y = (1<<right) - col.y;

		highPart = col.x << right;
		lowPart = col.y;

		return (highPart + lowPart) << (16 - m_Quantization);
	}
/*
	Color Split::EncodeValue(uint16_t val)
	{
		int m_AlgoBits = 8;
		val >>= (16 - m_Quantization);
		Color ret;
		uint8_t highPart, lowPart;

		highPart = val >> (m_Quantization - 8);
		if (highPart % 2 == 0)
			lowPart = val % (1 << (m_Quantization - 8));
		else
			lowPart = (1<<(m_Quantization - 8)) - (val % (1 << (m_Quantization - 8)));

		ret[0] = highPart; ret[1] = lowPart << (16 - m_Quantization); ret[2] = 0;
		return ret;
	}

	uint16_t Split::DecodeValue(Color col)
	{
		uint16_t highPart = col.x;
		uint16_t lowPart = col.y >> (16 - m_Quantization);
		uint16_t delta = 0;

		int m = highPart % 2;

		switch (m) {
		case 0:
			delta = lowPart;
			break;
		case 1:
			delta = (1 << (m_Quantization - 8)) - lowPart;
			break;
		}

		return ((highPart << (m_Quantization - 8)) + delta) << (16 - m_Quantization);
	}
	*/
}
