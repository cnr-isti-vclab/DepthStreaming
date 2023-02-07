#include "Morton.h"

// Credits for Morton convertions: https://github.com/davemc0/DMcTools/blob/main/Math/SpaceFillCurve.h

namespace DStream
{
	Morton::Morton(uint8_t quantization, uint8_t algoBits) : Coder(quantization, algoBits) {}

	Color Morton::EncodeValue(uint16_t val)
	{
		Color ret;
		ret[0] = 0; ret[1] = 0; ret[2] = 0;

		for (unsigned int i = 0; i <= std::min((uint8_t)6, m_AlgoBits); ++i) {
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
		int codex = 0, codey = 0, codez = 0;

		const int nbits2 = 2 * m_AlgoBits;

		for (int i = 0, andbit = 1; i < nbits2; i += 2, andbit <<= 1) {
			codex |= (int)(col.x & andbit) << i;
			codey |= (int)(col.y & andbit) << i;
			codez |= (int)(col.z & andbit) << i;
		}

		return ((codez << 2) | (codey << 1) | codex);
	}
}