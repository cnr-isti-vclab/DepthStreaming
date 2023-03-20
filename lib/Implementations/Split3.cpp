#include "Split3.h"
#include <math.h>

namespace DStream
{
	Split3::Split3(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistribution)
		: Coder(quantization, algoBits, channelDistribution) {}


	Color Split3::EncodeValue(uint16_t val)
	{
		Color ret;
		uint32_t left = m_ChannelDistribution[0], mid = m_ChannelDistribution[1], right = m_ChannelDistribution[2];

		ret.x = val >> (right + mid);
		ret.y = (val >> right) & ((1 << mid) - 1);
		ret.z = val & ((1 << right) - 1);

		if ((ret.x & 0x1) == 1)
		{
			ret.y = (1 << mid) - ret.y - 1;
			ret.z = (1 << right) - ret.z - 1;
		}

		for (uint32_t i = 0; i < 3; i++)
			ret[i] <<= (8 - m_ChannelDistribution[i]);

		std::swap(ret.x, ret.y);
		return ret;
	}

	uint16_t Split3::DecodeValue(Color col)
	{
		std::swap(col.x, col.y);
		uint32_t left = m_ChannelDistribution[0], mid = m_ChannelDistribution[1], right = m_ChannelDistribution[2];

		for (uint32_t i = 0; i < 3; i++)
			col[i] = std::round((float)col[i] / (1 << (8 - m_ChannelDistribution[i])));

		if ((col.x & 0x1) == 1)
		{
			col.y = (1 << mid) - col.y - 1;
			col.z = (1 << right) - col.z - 1;
		}

		return (col.x << (mid + right)) + (col.y << right) + col.z;
	}
}
