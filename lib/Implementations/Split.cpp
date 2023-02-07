#include "Split.h"

namespace DStream
{
	Split::Split(uint8_t quantization, uint8_t algoBits) : Coder(quantization, algoBits) {}

	Color Split::EncodeValue(uint16_t val)
	{
		Color ret;
		uint8_t highPart, lowPart;

		highPart = val >> 8;
		if (highPart % 2 == 0) lowPart = val % 256;
		else lowPart = 255 - (val % 256);

		ret[0] = highPart; ret[1] = lowPart; ret[2] = 0.0f;
		return ret;
	}

	uint16_t Split::DecodeValue(Color col)
	{
		uint16_t highPart = col.x;
		uint16_t lowPart = col.y;
		uint16_t delta = 0;

		int m = highPart % 2;

		switch (m) {
		case 0:
			delta = lowPart;
			break;
		case 1:
			delta = 255 - lowPart;
			break;
		}

		return highPart * 256 + delta;
	}
}