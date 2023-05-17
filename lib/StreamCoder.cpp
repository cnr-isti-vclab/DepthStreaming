#include <StreamCoder.h>
#include <Implementations/Hilbert.h>
#include <Implementations/HilbertDebug.h>
#include <Implementations/Hue.h>
#include <Implementations/Packed2.h>
#include <Implementations/Packed3.h>
#include <Implementations/Split2.h>
#include <Implementations/Split3.h>
#include <Implementations/Phase.h>
#include <Implementations/Triangle.h>

#include <../benchmark/ImageWriter.h>
// [TMP]
#include <iostream>
#include <fstream>
#include <sstream>

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

static void NormalizeAdvance(std::vector<uint16_t>& advances, uint32_t range)
{
	TransposeAdvanceToRange(advances, range);

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
	TransposeAdvanceToRange(nonZeros, range - zeroes);

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
	if (sum != range && advances[maxIdx] > (sum - range))
		advances[maxIdx] -= (sum - range);
}

namespace DStream
{
	template class StreamCoder<Hilbert>;
	template class StreamCoder<HilbertDebug>;
	template class StreamCoder<Morton>;
	template class StreamCoder<Split2>;
	template class StreamCoder<Split3>;
	template class StreamCoder<Packed2>;
	template class StreamCoder<Packed3>;
	template class StreamCoder<Phase>;
	template class StreamCoder<Hue>;
	template class StreamCoder<Triangle>;

	template <typename CoderImplementation>
	StreamCoder<CoderImplementation>::StreamCoder(uint8_t quantization, bool enlarge, bool interpolate, uint8_t algoBits, 
		std::vector<uint8_t> channelDistribution, bool useTables /* = true*/)
	{
		m_UseTables = useTables;
		m_Enlarge = enlarge;
		m_Interpolate = interpolate;

		m_Implementation = CoderImplementation(quantization, algoBits, channelDistribution);
		m_AlgoBits = m_Implementation.GetAlgoBits();
		m_SegmentBits = m_Implementation.GetSegmentBits();
		
		if (enlarge)
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
			Color prev, curr;
			for (uint32_t i = 0; i < nElements; i++)
			{
				if (m_Interpolate)
				{
					uint16_t frac = source[i] & ((1 << m_SegmentBits) - 1);
					Color prev = m_Implementation.EncodeValue(source[i] >> m_SegmentBits);
					Color curr = m_Implementation.EncodeValue((source[i] + 1) >> m_SegmentBits);

					dest[i] = InterpolateColor(prev, curr, (float)frac / (1 << m_SegmentBits));
				}
				else
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
				dest[i] = m_DecodingTable[inCols[i].x * usedBits2 + inCols[i].y * usedBits + inCols[i].z];
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
					m_DecodingTable[i * maxAlgoBitsValue * maxAlgoBitsValue + j * maxAlgoBitsValue + k] = m_Implementation.DecodeValue(c);
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
		// CHECK SIDE
		uint16_t* table = new uint16_t[side * side * side];

		for (uint16_t i = 0; i < side; i++)
			for (uint16_t j = 0; j < side; j++)
				for (uint16_t k = 0; k < side; k++)
				{
					uint16_t val = m_Implementation.DecodeValue(Color((uint8_t)i, (uint8_t)j, (uint8_t)k));
					table[i * side * side + j * side + k] = val;
				}

		// Compute spacing tables
		for (uint32_t e = 0; e < 3; e++)
		{
			// Init error vector
			AxisErrors dummy;
			std::vector<uint16_t> errors = GetErrorVector(table, side, e, dummy);

			if (side == 256)
				errors.assign(errors.size(), 1);
			else
				NormalizeAdvance(errors, (1 << 8) - 1);

			uint32_t sum = 0;
			for (uint32_t i = 0; i < errors.size(); i++)
				sum += errors[i];

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

		delete[] table;
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::SetSpacingTables(SpacingTable tables)
	{
		m_SpacingTable = tables;
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::SetEncodingTable(const std::vector<Color>& table)
	{
		m_EncodingTable = table;
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::SetDecodingTable(const std::vector<uint16_t>& table, uint32_t tableSideX, uint32_t tableSideY, uint32_t tableSideZ)
	{
		m_DecodingTable = table;
	}

	template<class CoderImplementation>
	std::vector<uint16_t> StreamCoder<CoderImplementation>::GetErrorVector(uint16_t* table, uint32_t tableSide, uint8_t axis, AxisErrors& errs, uint8_t amount)
	{
		uint32_t errSize = tableSide - 1;
		std::vector<uint16_t> ret(tableSide, 0);

		for (uint32_t i = 0; i < tableSide; i++)
		{
			for (uint32_t j = 0; j < tableSide; j++)
			{
				for (uint32_t k = 0; k < tableSide - 1; k++)
				{
					int tableLeft, tableRight;

					switch (axis)
					{
					case 0:
						tableLeft = table[ i* tableSide * tableSide + j*tableSide + k];
						tableRight = table[i* tableSide * tableSide + j*tableSide + (k + amount)];
						break;
					case 1:
						tableLeft = table[i* tableSide * tableSide + k * tableSide + j];
						tableRight = table[i* tableSide * tableSide + (k + amount) * tableSide + j];
						break;
					case 2:
						tableLeft = table[k* tableSide * tableSide + i * tableSide + j];
						tableRight = table[(k+amount)* tableSide * tableSide + i * tableSide + j];
						break;
					}

					if (std::abs(tableLeft-tableRight)< 60000)
						ret[k] = std::max<int>(ret[k], std::abs(tableLeft - tableRight));
				}
			}
		}

		for (uint32_t i = 0; i < ret.size(); i++)
		{
			errs.AvgErr += ret[i];
			errs.QuadErr += ret[i] * ret[i];
			errs.MaxErr = std::max<uint32_t>(ret[i], errs.MaxErr);
		}

		errs.AvgErr /= ret.size();
		errs.QuadErr /= ret.size();

		return ret;
	}
}
