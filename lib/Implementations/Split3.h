#pragma once

#include "../Coder.h"

namespace DStream
{
	class Split3 : public Coder
	{
	public:
		Split3() = default;
		Split3(uint8_t algoBits, std::vector<uint8_t> channelDistribution);

		inline bool SupportsInterpolation() { return false; }

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
	};
}