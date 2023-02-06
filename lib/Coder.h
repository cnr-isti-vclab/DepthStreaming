#pragma once
#include <cstdint>
#include <DataStructs/Vec3.h>

namespace DStream
{
	class Coder
	{
	public:
		Coder() = default;
		Coder(uint8_t quantization, bool enlarge, uint8_t algoBits);

	protected:
		uint8_t m_Quantization;
		uint8_t m_AlgoBits;
		bool m_Enlarge;
	};
}
