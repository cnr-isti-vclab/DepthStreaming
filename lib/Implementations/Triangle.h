#pragma once

#include "../Coder.h"

namespace DStream
{
	class Triangle : public Coder
	{
	public:
		Triangle() = default;
		Triangle(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistribution);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
		virtual uint16_t DecodeValue(Color value, bool simple) override { return 0; }

	};
}