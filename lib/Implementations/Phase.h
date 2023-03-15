#pragma once

#include "../Coder.h"

namespace DStream
{
	class Phase : public Coder
	{
	public:
		Phase() = default;
		Phase(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistribution);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
	};
}