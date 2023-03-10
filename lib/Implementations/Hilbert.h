
#pragma once

#include <Implementations/Morton.h>

namespace DStream
{
	class Hilbert : public Coder
	{
	public:
		Hilbert() = default;
		Hilbert(uint8_t quantization, uint8_t algoBits);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);

		inline uint8_t GetEnlargeBits() { return m_AlgoBits + m_SegmentBits; }
		inline uint8_t GetUsedBits() { return m_AlgoBits + m_SegmentBits; }

	private:
		void SubdivideValue(Color& value, Color& nextValue, uint8_t fract);
		uint16_t UnsubdivideValue(Color& value, Color& nextValue);

		void TransposeToHilbertCoords(Color& col);
		void TransposeFromHilbertCoords(Color& col);

	private:
		uint8_t m_SegmentBits;
		Morton m_Morton;
	};
}