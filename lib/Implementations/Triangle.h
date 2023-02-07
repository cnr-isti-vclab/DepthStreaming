#pragma once

#include "../Coder.h"

namespace DStream
{
	class Triangle : public Coder
	{
	public:
		Triangle() = default;
		Triangle(uint8_t quantization, bool enlarge, uint8_t algoBits);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
	};
}