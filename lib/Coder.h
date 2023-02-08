#pragma once
#include <cstdint>
#include <DataStructs/Vec3.h>

namespace DStream
{
	class Coder
	{
	public:
		Coder() = default;
		Coder(uint8_t quantization, uint8_t algoBits) : m_Quantization(quantization), m_AlgoBits(algoBits) {}

		inline uint8_t GetUsedBits() { return m_AlgoBits; }
		inline uint8_t GetAlgoBits() { return m_AlgoBits; }
		inline uint8_t GetEnlargeBits() { return m_AlgoBits; }
		inline uint8_t GetQuantization() { return m_Quantization; }

	protected:
		uint8_t m_Quantization;
		uint8_t m_AlgoBits;
	};
}
