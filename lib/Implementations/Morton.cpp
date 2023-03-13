#include "Morton.h"

// Credits for Morton convertions: https://github.com/davemc0/DMcTools/blob/main/Math/SpaceFillCurve.h

namespace DStream
{
	Morton::Morton(uint8_t quantization, uint8_t algoBits, bool hilbert) : Coder(quantization, algoBits), m_ForHilbert(hilbert) {}

	Color Morton::EncodeValue(uint16_t val)
	{
		Color ret;
		ret[0] = 0; ret[1] = 0; ret[2] = 0;

		uint8_t algoBits;
		if (m_ForHilbert)
			algoBits = m_AlgoBits;
		else
		{
			algoBits = 6;
			val <<= (16 - m_Quantization);
		}

		for (unsigned int i = 0; i <= algoBits; ++i) {
			uint8_t selector = 1;
			unsigned int shift_selector = 3 * i;
			unsigned int shiftback = 2 * i;

			ret[0] |= (val & (selector << shift_selector)) >> (shiftback);
			ret[1] |= (val & (selector << (shift_selector + 1))) >> (shiftback + 1);
			ret[2] |= (val & (selector << (shift_selector + 2))) >> (shiftback + 2);
		}
		return ret;
	}

	uint16_t Morton::DecodeValue(Color col)
	{
		int codex = 0, codey = 0, codez = 0, nbits2;

		if (m_ForHilbert)
			nbits2 = 2 * m_AlgoBits;
		else
			nbits2 = 12;

		for (int i = 0, andbit = 1; i < nbits2; i += 2, andbit <<= 1) {
			codex |= (int)(col.x & andbit) << i;
			codey |= (int)(col.y & andbit) << i;
			codez |= (int)(col.z & andbit) << i;
		}

		uint16_t ret = ((codez << 2) | (codey << 1) | codex);
		return ret;
	}
}