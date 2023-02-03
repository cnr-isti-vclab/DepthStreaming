#pragma once
#include <cstdint>

class AlgorithmImplementation
{
public:
	AlgorithmImplementation(uint8_t quantization, bool enlarge, uint8_t algoBits);
protected:
	uint8_t m_Quantization;
	uint8_t m_AlgoBits;
	uint8_t m_SegmentBits;
	bool m_Enlarge;
};