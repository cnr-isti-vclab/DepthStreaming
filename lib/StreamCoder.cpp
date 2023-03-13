#include <StreamCoder.h>
#include <Implementations/Hilbert.h>
#include <Implementations/Hue.h>
#include <Implementations/Packed.h>
#include <Implementations/Split.h>
#include <Implementations/Phase.h>
#include <Implementations/Triangle.h>

#include <iostream>

#include <math.h>
#include <string.h>

static void TransposeAdvanceToRange(std::vector<uint16_t>& vec, uint16_t rangeMax)
{
	uint32_t errorSum = 0;
	for (uint32_t i = 0; i < vec.size(); i++)
		errorSum += vec[i];

	for (uint32_t i = 0; i < vec.size(); i++)
		vec[i] = std::round(((float)vec[i] / errorSum) * rangeMax);
}

static void NormalizeAdvance(std::vector<uint16_t>& advances)
{
	TransposeAdvanceToRange(advances, 255);

	// Count zeros, store non zeros
	std::vector<uint16_t> nonZeros;
	uint16_t zeroes = 0;
	for (uint16_t i = 0; i < advances.size(); i++)
	{
		if (advances[i] == 0)
			zeroes++;
		else
			nonZeros.push_back(advances[i]);
	}

	// Make sure the sum of the non zero elements is 256 - nZeros (we'll set the zeros to ones)
	TransposeAdvanceToRange(nonZeros, 255 - zeroes);

	// Update the non zero elements, turn the zeros into ones
	uint32_t nonzeroIdx = 0;
	uint32_t max = 0;
	uint32_t maxIdx;
	uint32_t sum = 0;

	for (uint32_t i = 0; i < advances.size(); i++)
	{
		if (advances[i] != 0)
		{
			advances[i] = nonZeros[nonzeroIdx];
			nonzeroIdx++;
		}
		else
			advances[i] = 1;

		sum += advances[i];
		max = std::max<uint32_t>(max, advances[i]);
		if (max == advances[i])
			maxIdx = i;
	}

	// Remove from the max in case the sum still isn't 256
	if (sum != 255)
		advances[maxIdx] -= (sum - 255);
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

	template <typename CoderImplementation>
	StreamCoder<CoderImplementation>::StreamCoder(uint8_t quantization, bool enlarge, uint8_t algoBits, bool useTables /* = true*/)
	{
		m_UseTables = useTables;
		m_Enlarge = enlarge;
		m_Implementation = CoderImplementation(quantization, algoBits);
		
		GenerateSpacingTables();
		if (useTables)
			GenerateCodingTables();
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::Encode(const uint16_t* source, Color* dest, uint32_t nElements)
	{
		if (m_UseTables)
		{
			for (uint32_t i = 0; i < nElements; i++)
				dest[i] = m_EncodingTable[source[i]];
		}
		else
		{
			for (uint32_t i = 0; i < nElements; i++)
				dest[i] = m_Implementation.EncodeValue(source[i]);

			if (m_Enlarge)
			{
				Color* enlarged = new Color[nElements];
				Enlarge(dest, enlarged, nElements);
				memcpy(dest, enlarged, nElements * 3);
				delete[] enlarged;
			}
		}
	}


	// Interpolate values from the table if necessary
	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::Decode(const Color* source, uint16_t* dest, uint32_t nElements)
	{
		Color* inCols = nullptr;

		if (m_Enlarge)
		{
			inCols = new Color[nElements];
			Shrink(source, inCols, nElements);
		}
		else
			inCols = (Color*)source;

		if (m_UseTables)
		{
			uint32_t usedBits = 1 << m_Implementation.GetUsedBits();
			uint32_t usedBits2 = usedBits * usedBits;
			for (uint32_t i = 0; i < nElements; i++)
				dest[i] = m_DecodingTable[inCols[i].x * usedBits2 + inCols[i].y*usedBits + inCols[i].z];
		}
		else
		{
			for (uint32_t i = 0; i < nElements; i++)
				dest[i] = m_Implementation.DecodeValue(inCols[i]);
		}

		if (m_Enlarge)
			delete[] inCols;
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::GenerateCodingTables()
	{
		uint32_t maxQuantizationValue = (1 << m_Implementation.GetQuantization());
		uint32_t maxAlgoBitsValue = (1 << m_Implementation.GetUsedBits());

		m_DecodingTable.resize(maxAlgoBitsValue * maxAlgoBitsValue * maxAlgoBitsValue);
		for (uint32_t i = 0; i < maxAlgoBitsValue; i++)
		{
			for (uint32_t j = 0; j < maxAlgoBitsValue; j++)
			{
				for (uint32_t k = 0; k < maxAlgoBitsValue; k++)
				{
					Color c = { (uint8_t)i, (uint8_t)j, (uint8_t)k };
					m_DecodingTable[i * maxAlgoBitsValue * maxAlgoBitsValue + j * maxAlgoBitsValue + k] =
						m_Implementation.DecodeValue(c);
				}
			}
		}

		m_EncodingTable.resize(maxQuantizationValue);
		for (uint32_t i = 0; i < maxQuantizationValue; i++)
		{
			Color c = m_Implementation.EncodeValue(i);
			if (m_Enlarge)
				Enlarge(&c, &c, 1);
			m_EncodingTable[i] = c;
		}
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::GenerateSpacingTables()
	{
		// Init tables
		uint32_t side = 1 << m_Implementation.GetEnlargeBits();
		// Init table memory
		uint16_t* table = new uint16_t[side * side * side];

		// [OPTIMIZABLE] Compute decoding table
		for (uint16_t i = 0; i < side; i++)
			for (uint16_t j = 0; j < side; j++)
				for (uint16_t k = 0; k < side; k++)
					table[i*side*side + j*side + k] = m_Implementation.DecodeValue(Color((uint8_t)i, (uint8_t)j, (uint8_t)k));

		// Compute spacing tables
		for (uint32_t e = 0; e < 3; e++)
		{
			// Init error vector
			std::vector<uint16_t> errors = GetErrorVector(table, side, e);
			if (errors.size() < 255)
				NormalizeAdvance(errors);
			else
				for (uint32_t i = 0; i < 255; i++)
					errors[i] = 1;

			// Create spacings based on that vector
			uint32_t nextNumber = 0;

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

		if (m_SpacingTable.Shrink[0].size() != 256 || m_SpacingTable.Shrink[1].size() != 256 || m_SpacingTable.Shrink[2].size() != 256)
		{
			std::cout << "Q: " << (int)m_Implementation.GetQuantization() << ", algo: " << (int)m_Implementation.GetAlgoBits() << std::endl;
			std::cout << "Table size: " << m_SpacingTable.Shrink[0].size() << "," << m_SpacingTable.Shrink[1].size() << "," << m_SpacingTable.Shrink[2].size() << std::endl;
		}

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

					switch (axis)
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

			/*if (ret[k] > 40000)
				ret[k] /= 128;*/
		}

		return ret;
	}
}
