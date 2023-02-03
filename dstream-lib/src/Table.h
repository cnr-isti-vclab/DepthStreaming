#pragma once

#include <cstdint>
#include <vector>

namespace DStream
{
	struct SpacingTable
	{
		std::vector<uint16_t> Enlarge[3];
		std::vector<uint16_t> Shrink[3];
	};
}
