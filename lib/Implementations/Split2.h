#pragma once

#include "../Coder.h"

namespace DStream
{
	class Split2 : public Coder
	{
	public:
		Split2() = default;
		Split2(uint8_t algoBits, std::vector<uint8_t> channelDistribution);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
	};
}