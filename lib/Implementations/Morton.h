
#pragma once

#include "../Coder.h"

namespace DStream
{
	class Morton : public Coder
	{
	public:
		Morton() = default;
		Morton(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistribution, bool hilbert = false);

		inline uint8_t GetEnlargeBits() { return std::min<uint8_t>(6, m_AlgoBits); }

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
		virtual uint16_t DecodeValue(Color value, bool simple) override { return 0; }

	private:
		bool m_ForHilbert;
	};
}