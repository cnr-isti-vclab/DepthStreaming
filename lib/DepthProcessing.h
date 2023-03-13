#pragma once

#include <cstdint>

namespace DStream
{
	class DepthProcessing
	{
	public:
		static void DenoiseMedian(uint16_t* source, uint32_t width, uint32_t height, uint32_t threshold, int halfWindow = 1);

		static void Quantize(uint16_t* dest, uint16_t* source, uint8_t q, uint32_t nElements);
		static void Dequantize(uint16_t* dest, uint16_t* source, uint8_t q, uint32_t nElements);
	};
}