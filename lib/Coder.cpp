#include <Coder.h>

namespace DStream
{
	Coder::Coder(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistribution) :
		m_Quantization(quantization), m_AlgoBits(algoBits), m_ChannelDistribution(channelDistribution) {}
}