
#pragma once

#include <Implementations/Morton.h>

namespace DStream
{
	class Hilbert3 : public Coder
	{
	public:
		Hilbert3() = default;
		Hilbert3(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistributions);


		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
		virtual uint16_t DecodeValue(Color value, bool simple) { return 0; }

		inline uint8_t GetEnlargeBits() { return m_AlgoBits + m_SegmentBits; }
		inline uint8_t GetUsedBits() { return m_AlgoBits + m_SegmentBits; }

	private:
		void TransposeToHilbertCoords(Color& col);
		void TransposeFromHilbertCoords(Color& col);

	private:
		uint8_t m_SegmentBits;
		Morton m_Morton;
	};
}