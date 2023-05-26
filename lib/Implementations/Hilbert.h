
#pragma once

#include <Implementations/Morton.h>

namespace DStream
{
	class Hilbert : public Coder
	{
	public:
		Hilbert() = default;
		Hilbert(uint8_t algoBits, std::vector<uint8_t> channelDistributions);

		Color EncodeValue(uint16_t value);
		uint16_t DecodeValue(Color value);

	private:
		void TransposeToHilbertCoords(Color& col);
		void TransposeFromHilbertCoords(Color& col);

	private:
		Morton m_Morton;
	};
}