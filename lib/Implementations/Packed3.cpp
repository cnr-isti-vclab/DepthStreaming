#include "Packed3.h"

namespace DStream
{
	Packed3::Packed3(uint8_t algoBits, std::vector<uint8_t> channelDistribution) 
		: Coder(algoBits,  channelDistribution) {}

	Color Packed3::EncodeValue(uint16_t val)
	{
		Color ret;
		uint32_t left = m_ChannelDistribution[0], mid = m_ChannelDistribution[1], right = m_ChannelDistribution[2];

		ret.x = (val >> (mid + right));
		ret.y = (val >> right) & ((1 << mid)-1);
		ret.z = val & ((1 << right)-1);

		for (uint32_t i = 0; i < 3; i++)
			ret[i] <<= (8 - m_ChannelDistribution[i]);
		return ret;
	}

	uint16_t Packed3::DecodeValue(Color col)
	{
		uint8_t leftPart, midPart, rightPart;
		uint32_t left = m_ChannelDistribution[0], mid = m_ChannelDistribution[1], right = m_ChannelDistribution[2];

		for (uint32_t i = 0; i < 3; i++)
			col[i] = std::round((float)col[i] / (1<<(8 - m_ChannelDistribution[i])));

		leftPart = col.x;
		midPart = col.y;
		rightPart = col.z;

		return (leftPart << (mid + right)) + (midPart << right) + rightPart;
	}
}