#include <StreamCoder.h>
#include <Implementations/Hilbert.h>
#include <Implementations/Hue.h>
#include <Implementations/Packed.h>
#include <Implementations/Split.h>
#include <Implementations/Phase.h>
#include <Implementations/Triangle.h>

#include <iostream>

static void TransposeAdvanceToRange(std::vector<uint16_t>& vec, uint16_t rangeMax)
{
	uint32_t errorSum = 0;
	// Scale advance vector from 0 to 256
	for (uint32_t i = 0; i < vec.size(); i++)
		errorSum += vec[i];

	for (uint32_t i = 0; i < vec.size(); i++)
		vec[i] = std::round(((float)vec[i] / errorSum) * rangeMax);
}

static void RemoveZerosFromAdvance(std::vector<uint16_t>& advances)
{
	std::vector<uint16_t> nonZeros;
	uint16_t zeroes = 0;
	for (uint16_t i = 0; i < advances.size(); i++)
	{
		if (advances[i] == 0)
		{
			advances[i] = 1;
			zeroes++;
		}
		else
			nonZeros.push_back(advances[i]);
	}

	TransposeAdvanceToRange(nonZeros, 256 - zeroes);

	uint32_t nonzeroIdx = 0;
	for (uint32_t i = 0; i < advances.size(); i++)
	{
		if (advances[i] != 1)
		{
			advances[i] = nonZeros[nonzeroIdx];
			nonzeroIdx++;
		}
	}
}

namespace DStream
{
	template class StreamCoder<Hilbert>;
	template class StreamCoder<Morton>;
	template class StreamCoder<Split>;
	template class StreamCoder<Packed>;
	template class StreamCoder<Phase>;
	template class StreamCoder<Hue>;
	template class StreamCoder<Triangle>;

	template <typename CoderImplementation
			  /*, typename std::enable_if<std::is_base_of_v<Coder, CoderImplementation>, bool>::type = true */ >
	StreamCoder<CoderImplementation>::StreamCoder(uint8_t quantization, bool enlarge, uint8_t algoBits, bool useTables /* = true*/)
	{
		m_UseTables = useTables;
		m_Enlarge = enlarge;
		m_Implementation = CoderImplementation(quantization, algoBits);
		if (useTables)
			GenerateCodingTables();
		GenerateSpacingTables();
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::Encode(const uint16_t* source, Color* dest, uint32_t nElements)
	{
		if (m_UseTables)
		{

		}
		else
		{
			for (uint32_t i = 0; i < nElements; i++)
				dest[i] = m_Implementation.EncodeValue(source[i]);
		}

		
		if (m_Enlarge)
		{
			Color* enlarged = new Color[nElements];
			Enlarge(dest, enlarged, nElements);
			memcpy(dest, enlarged, nElements * 3);
			delete[] enlarged;
		}
	}


	// Interpolate values from the table if necessary
	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::Decode(const Color* source, uint16_t* dest, uint32_t nElements)
	{
		Color* inCols = new Color[nElements];

		if (m_Enlarge)
			Shrink(source, inCols, nElements);
		else
			memcpy(inCols, source, nElements * 3);

		if (m_UseTables)
		{

		}
		else
		{
			for (uint32_t i = 0; i < nElements; i++)
				dest[i] = m_Implementation.DecodeValue(inCols[i]);
		}

		delete[] inCols;
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::GenerateCodingTables()
	{

	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::GenerateSpacingTables()
	{
		// Init tables
		uint32_t side = 1 << m_Implementation.GetUsedBits();

		// Init table memory
		uint16_t* table = new uint16_t[side * side * side];

		// [FUNCTION] Compute decoding table
		for (uint16_t i = 0; i < side; i++)
			for (uint16_t j = 0; j < side; j++)
				for (uint16_t k = 0; k < side; k++)
					table[i*side*side + j*side + k] = m_Implementation.DecodeValue(Color((uint8_t)i, (uint8_t)j, (uint8_t)k));

		// Compute spacing tables
		for (uint32_t e = 0; e < 3; e++)
		{
			// Init error vector
			std::vector<uint16_t> errors = GetErrorVector(table, side, e);

			// Create spacings based on that vector
			uint32_t nextNumber = 0;

			TransposeAdvanceToRange(errors, 256);
			RemoveZerosFromAdvance(errors);
			TransposeAdvanceToRange(errors, 256);

			int sum = 0;
			for (uint32_t i = 0; i < errors.size(); i++)
				sum += errors[i];

			m_SpacingTable.Enlarge[e].push_back(0);
			m_SpacingTable.Shrink[e].push_back(0);

			for (uint32_t i = 1; i < errors.size() + 1; i++)
			{
				float advance = errors[i - 1];
				nextNumber += advance;

				if (advance == 1)
				{
					m_SpacingTable.Shrink[e].push_back(i);
					m_SpacingTable.Enlarge[e].push_back(nextNumber);
				}
				else
				{
					m_SpacingTable.Enlarge[e].push_back(nextNumber);

					for (uint32_t j = 0; j < std::floor(advance / 2); j++)
						m_SpacingTable.Shrink[e].push_back(i - 1);
					for (uint32_t j = 0; j < std::ceil(advance / 2); j++)
						m_SpacingTable.Shrink[e].push_back(i);
				}
			}
		}

		std::cout << "SPACING TABLE" << std::endl;
		for (uint32_t i = 0; i < 3; i++)
			std::cout << "Enlarge " << i << " size: " << m_SpacingTable.Enlarge[i].size() << std::endl;
		for (uint32_t i = 0; i < 3; i++)
			std::cout << "Shrink " << i << " size: " << m_SpacingTable.Shrink[i].size() << std::endl;
		std::cout << std::endl;

		delete[] table;
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::SetSpacingTables(SpacingTable tables)
	{

	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::SetEncodingTable(const std::vector<Color>& table)
	{

	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::SetDecodingTable(const std::vector<uint16_t>& table, uint32_t tableSideX, uint32_t tableSideY, uint32_t tableSideZ)
	{

	}

	template<class CoderImplementation>
	std::vector<uint16_t> StreamCoder<CoderImplementation>::GetErrorVector(uint16_t* table, uint32_t tableSide, uint8_t axis)
	{
		std::vector<uint16_t> ret(tableSide - 1);
		for (uint32_t k = 0; k < tableSide - 1; k++)
		{
			uint16_t max = 0;
			for (uint32_t i = 0; i < tableSide; i++)
			{
				for (uint32_t j = 0; j < tableSide; j++)
				{
					switch (i)
					{
					case 0:
						max = std::max<uint16_t>(max, abs(table[k * tableSide * tableSide + i * tableSide + j] - 
							table[(k + 1) * tableSide * tableSide + i * tableSide + j]));
						break;
					case 1:
						max = std::max<uint16_t>(max, abs(table[i * tableSide * tableSide + k * tableSide + j] - 
							table[i * tableSide * tableSide + (k+1) * tableSide + j]));
						break;
					case 2:
						max = std::max<uint16_t>(max, abs(table[i * tableSide * tableSide + j * tableSide + k] - 
							table[i * tableSide * tableSide + j * tableSide + k + 1]));
						break;
					}
				}
			}
			ret[k] = max;
		}

		return ret;
	}
}