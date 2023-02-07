
#pragma once

#include "../Coder.h"

namespace DStream
{
	class Morton : public Coder
	{
	public:
		Morton() = default;
		Morton(uint8_t quantization, bool enlarge, uint8_t algoBits);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
	};
}