#include <Coder.h>

namespace DStream
{
	Coder::Coder(uint8_t algoBits, std::vector<uint8_t> channelDistribution) : m_AlgoBits(algoBits), m_ChannelDistribution(channelDistribution) {}
}