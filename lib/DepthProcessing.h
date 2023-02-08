#pragma once

#include <cstdint>

namespace DStream
{
	class DepthProcessing
	{
	public:
		static void DenoiseMedian(uint16_t* source, uint32_t width, uint32_t height, uint32_t threshold, uint32_t halfWindow = 1);
		static void DenoiseMedianExclusive(uint16_t* source, uint32_t width, uint32_t height, uint32_t threshold, uint32_t halfWindow = 1);

		static uint16_t* Quantize(uint16_t* source, uint8_t q, uint32_t nElements);
	};
}