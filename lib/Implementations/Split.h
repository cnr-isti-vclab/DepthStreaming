#pragma once

#include "../Coder.h"

namespace DStream
{
	class Split : public Coder
	{
	public:
		Split() = default;
		Split(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistribution);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);

		inline uint8_t GetEnlargeBits() { return 8; }
		inline uint8_t GetUsedBits() { return 8; }
	};
}