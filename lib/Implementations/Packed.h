#pragma once

#include "../Coder.h"

namespace DStream
{
	class Packed : public Coder
	{
	public:
		Packed() = default;
		Packed(uint8_t quantization, uint8_t algoBits);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);

		uint8_t GetUsedBits() { return 8; }
		uint8_t GetEnlargeBits() { return 8; }
	};
}