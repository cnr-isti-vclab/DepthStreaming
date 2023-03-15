#pragma once

#include <cstdint>
#include <vector>

namespace DStream
{
	struct SpacingTable
	{
		std::vector<uint8_t> Enlarge[3];
		std::vector<uint8_t> Shrink[3];
	};
}
