#pragma once

#include "../Coder.h"

namespace DStream
{
	class Packed3 : public Coder
	{
	public:
		Packed3() = default;
		Packed3(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistribution);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
		virtual uint16_t DecodeValue(Color value, bool simple) override { return 0; }

		uint8_t GetUsedBits() { return 8; }
		uint8_t GetEnlargeBits() { return 8; }
	};
}