
#pragma once

#include "../Coder.h"

namespace DStream
{
	class Morton : public Coder
	{
	public:
		Morton() = default;
		Morton(uint8_t algoBits, std::vector<uint8_t> channelDistribution, bool hilbert = false);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);

	private:
		bool m_ForHilbert;
	};
}