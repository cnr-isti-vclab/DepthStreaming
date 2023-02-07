#include "Packed.h"

namespace DStream
{
	Packed::Packed(uint8_t quantization, uint8_t algoBits) : Coder(quantization, algoBits) {}

	Color Packed::EncodeValue(uint16_t val)
	{
		Color ret;
		uint32_t right = m_Quantization - m_AlgoBits;

		ret.x = (val >> (16 - m_AlgoBits)) << (8 - m_AlgoBits);
		ret.y = ((val >> (16 - m_Quantization)) & ((1 << right) - 1)) << (8 - right);

		return ret;
	}

	uint16_t Packed::DecodeValue(Color col)
	{
		Color copy = col;
		uint16_t highPart, lowPart;
		uint32_t right = m_Quantization - m_AlgoBits;

		copy.x >>= (8 - m_AlgoBits);
		copy.y >>= (8 - right);

		highPart = copy.x << (m_Quantization - m_AlgoBits);
		lowPart = copy.y;

		return (highPart + lowPart) << (16 - m_Quantization);
	}
}