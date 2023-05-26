#pragma once

#include "../Coder.h"

namespace DStream
{
	class Hue : public Coder
	{
	public:
		Hue() = default;
		Hue(uint8_t algoBits, std::vector<uint8_t> channelDistribution);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
	};
}