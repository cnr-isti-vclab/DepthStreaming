#pragma once
#include <cstdint>
#include <DataStructs/Table.h>
#include <DataStructs/Vec3.h>

namespace DStream
{
	class Coder
	{
	public:
		Coder() = default;
		Coder(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistribution);

		virtual uint16_t DecodeValue(Color value, bool simple) { return 0; };

		inline uint8_t GetUsedBits() { return m_AlgoBits; }
		inline uint8_t GetAlgoBits() { return m_AlgoBits; }
		inline uint8_t GetSegmentBits() { return m_SegmentBits; }
		inline uint8_t GetEnlargeBits() { return m_AlgoBits; }
		inline uint8_t GetQuantization() { return m_Quantization; }
		inline std::string GetName() { return m_Name; }

	protected:
		uint8_t m_Quantization;
		uint8_t m_AlgoBits;
		uint8_t m_SegmentBits;
		std::vector<uint8_t> m_ChannelDistribution;
		std::string m_Name;
	};
}
