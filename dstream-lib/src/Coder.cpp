#include "Coder.h"
#include "Vec3.h"

namespace DStream
{
	template<typename CoderImplementation, class>
	Coder<CoderImplementation>::Coder(uint8_t quantization, bool enlarge, uint8_t algoBits, bool useTables)
	{
		m_Implementation.
	}

	void Encode(const uint16_t* source, Color* dest, uint32_t nElements);
	// Interpolate values from the table if necessary
	void Decode(const Color* source, uint16_t* dest, uint32_t nElements);

	void GenerateTables();
	void SetSpacingTables(SpacingTable tables);
	void SetEncodingTable(const std::vector<Color>& table);
	void SetDecodingTable(const std::vector<uint16_t>& table, uint32_t tableSideX, uint32_t tableSideY, uint32_t tableSideZ);

	void Enlarge(Color* source, uint32_t nElements);
	void Shrink(Color* source, uint32_t nElements);
}