#pragma once

#include <cstdint>
#include <unordered_map>
#include <type_traits>

#include "AlgorithmImplementation.h"
#include "Table.h"
#include "Vec3.h"

namespace DStream
{
	template <	typename CoderImplementation/*,
				typename std::enable_if<std::is_base_of_v<AlgorithmImplementation, CoderImplementation>, bool>::type = true*/>
	class Coder
	{
		static_assert(std::is_base_of_v<AlgorithmImplementation, CoderImplementation>, "Template parameter of class Coder must derive from AlgorithmImplementation.");
	public:
		Coder();
		Coder(uint8_t quantization, bool enlarge, uint8_t algoBits, bool useTables);

		void Encode(const uint16_t* source, Color* dest, uint32_t nElements);
		// Interpolate values from the table if necessary
		void Decode(const Color* source, uint16_t* dest, uint32_t nElements);

		void GenerateTables();
		void SetSpacingTables(SpacingTable tables);
		void SetEncodingTable(const std::vector<Color>& table);
		void SetDecodingTable(const std::vector<uint16_t>& table, uint32_t tableSideX, uint32_t tableSideY, uint32_t tableSideZ);

		void Enlarge(Color* source, uint32_t nElements);
		void Shrink(Color* source, uint32_t nElements);

	private:
		CoderImplementation m_Implementation;

		bool m_UseTables;
		SpacingTable m_SpacingTable;
		std::unordered_map<uint16_t, Color> m_EncodingTable;
		std::unordered_map<Color, uint16_t> m_DecodingTable;
	};

}