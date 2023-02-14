#include "Split.h"

namespace DStream
{
	Split::Split(uint8_t quantization, uint8_t algoBits) : Coder(quantization, algoBits) {}

	Color Split::EncodeValue(uint16_t val)
	{
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
}