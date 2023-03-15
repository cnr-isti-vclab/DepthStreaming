#pragma once

#include <cstdint>

namespace DStream
{
	class DepthProcessing
	{
	public:
		static void DenoiseMedian(uint16_t* source, uint32_t width, uint32_t height, uint32_t threshold, int halfWindow = 1);

		static void Quantize(uint16_t* dest, float* source, uint8_t q, uint32_t nElements, float minHint = 1, float maxHint = 0);
		static void Quantize(uint16_t* dest, uint16_t* source, uint8_t q, uint32_t nElements, uint16_t minHint = 1, uint16_t maxHint = 0);
		static void Dequantize(uint16_t* dest, uint16_t* source, uint8_t currQ, uint32_t nElements, uint16_t minHint = 1, uint16_t maxHint = 0);
	};
}