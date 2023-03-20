#pragma once

#include "../Coder.h"

namespace DStream
{
	class Split3 : public Coder
	{
	public:
		Split3() = default;
		Split3(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistribution);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
		virtual uint16_t DecodeValue(Color value, bool simple) override { return 0; }

		inline uint8_t GetEnlargeBits() { return 8; }
		inline uint8_t GetUsedBits() { return 8; }
	};
}