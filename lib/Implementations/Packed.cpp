#include "Packed.h"

namespace DStream
{
	Packed::Packed(uint8_t quantization, uint8_t algoBits) : Coder(quantization, algoBits) {}

	Color Packed::EncodeValue(uint16_t val)
	{
		Color ret;
		uint32_t right = m_Quantization - m_AlgoBits;

		val >>= (16 - m_Quantization);

		ret.x = (val >> right) << (8 - m_AlgoBits);
		ret.y = (val & ((1 << right) - 1)) << (8 - right);
		ret.z = 0;

		return ret;
	}

	uint16_t Packed::DecodeValue(Color col)
	{
		uint16_t highPart, lowPart;
		uint32_t right = m_Quantization - m_AlgoBits;

		col.x >>= (8 - m_AlgoBits);
		col.y >>= (8 - right);

		highPart = col.x << right; 
		lowPart = col.y;

		return (highPart + lowPart) << (16 - m_Quantization);
	}
}