#include "Coder.h"
#include "Implementations/Hilbert.h"

namespace DStream
{
	static Coder<Hilbert> hilbertCoder;

	template <typename CoderImplementation
			  /*,typename std::enable_if<std::is_base_of_v<AlgorithmImplementation, CoderImplementation>, bool>::type = true */ >
	Coder<CoderImplementation>::Coder(uint8_t quantization, bool enlarge, uint8_t algoBits, bool useTables)
	{
		// constructor implementation
	}

	/*
	// Interpolate values from the table if necessary
	template<class CoderImplementation, class>
	void Coder<CoderImplementation>:: Decode(const Color* source, uint16_t* dest, uint32_t nElements)
	{

	}

	template<class CoderImplementation, class>
	void Coder<CoderImplementation>::GenerateTables()
	{

	}

	template<class CoderImplementation, class>
	void Coder<CoderImplementation>::SetSpacingTables(SpacingTable tables)
	{

	}

	template<class CoderImplementation, class>
	void Coder<CoderImplementation>::SetEncodingTable(const std::vector<Color>& table)
	{

	}

	template<class CoderImplementation, class>
	void Coder<CoderImplementation>::SetDecodingTable(const std::vector<uint16_t>& table, uint32_t tableSideX, uint32_t tableSideY, uint32_t tableSideZ)
	{

	}

	template<class CoderImplementation, class>
	void Coder<CoderImplementation>::Enlarge(Color* source, uint32_t nElements)
	{

	}

	template<class CoderImplementation, class>
	void Coder<CoderImplementation>::Shrink(Color* source, uint32_t nElements)
	{

	}
	*/
}