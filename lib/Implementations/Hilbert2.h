
#pragma once

#include <Implementations/Morton.h>

namespace DStream
{
	class Hilbert2 : public Coder
	{
	public:
		Hilbert2() = default;
		Hilbert2(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistributions);


		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
		virtual uint16_t DecodeValue(Color value, bool simple) { return 0; }

		inline uint8_t GetEnlargeBits() { return m_AlgoBits + m_SegmentBits; }
		inline uint8_t GetUsedBits() { return m_AlgoBits + m_SegmentBits; }

	private:
		Color Encode2D(uint16_t val);
		uint16_t Decode2D(Color c);

		void TransposeToHilbertCoords(Color& col);
		void TransposeFromHilbertCoords(Color& col);

	private:
		uint8_t m_SegmentBits;
	};
}