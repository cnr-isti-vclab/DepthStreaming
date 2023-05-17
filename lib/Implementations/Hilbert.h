
#pragma once

#include <Implementations/Morton.h>

namespace DStream
{
	class Hilbert : public Coder
	{
	public:
		Hilbert() = default;
		Hilbert(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistributions);


		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);
		virtual uint16_t DecodeValue(Color value, bool simple) { return 0; }

		inline uint8_t GetEnlargeBits() { 
			return m_AlgoBits + m_SegmentBits; 
		}
		inline uint8_t GetUsedBits() { return m_AlgoBits + m_SegmentBits; }

	private:
		void TransposeToHilbertCoords(Color& col);
		void TransposeFromHilbertCoords(Color& col);

		uint16_t MortonDecode(Vec3& col);

	private:
		Morton m_Morton;
	};
}