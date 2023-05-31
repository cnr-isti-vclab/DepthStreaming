#include <StreamCoder.h>
#include <Implementations/Hilbert.h>
#include <Implementations/Hue.h>
#include <Implementations/Phase.h>
#include <Implementations/Triangle.h>
#include <Implementations/Packed2.h>
#include <Implementations/Split2.h>
#include <Implementations/Packed3.h>
#include <Implementations/Split3.h>

static void TransposeAdvanceToRange(std::vector<uint16_t>& vec, uint16_t rangeMax)
{
	uint32_t currSum = 0;
	for (uint32_t i = 0; i < vec.size(); i++)
		currSum += vec[i];
		
	for (uint32_t i = 0; i < vec.size(); i++)
		vec[i] = std::round(((float)vec[i] / currSum) * rangeMax);
}

static void NormalizeAdvance(std::vector<uint16_t>& advances, uint32_t range, uint32_t minAdvance)
{
	TransposeAdvanceToRange(advances, range);

	// Count zeros, store non zeros
	std::vector<uint16_t> greaters;
	uint16_t nLessers = 0;
	for (uint16_t i = 0; i < advances.size(); i++)
	{
		if (advances[i] < minAdvance)
			nLessers++;
		else
			greaters.push_back(advances[i]);
	}

	// Make sure the sum of the non zero elements is 256 - nLessers*minAdvance (we'll set the lessers to be at least minAdvance)
	TransposeAdvanceToRange(greaters, range - (nLessers * minAdvance));

	// Update the non zero elements, turn the zeros into ones
	uint32_t nonzeroIdx = 0;
	uint32_t max = 0;
	uint32_t maxIdx;
	uint32_t sum = 0;

	for (uint32_t i = 0; i < advances.size(); i++)
	{
		if (advances[i] >= minAdvance)
		{
			advances[i] = greaters[nonzeroIdx];
			nonzeroIdx++;
		}
		else
			advances[i] = minAdvance;

		sum += advances[i];
		max = std::max<uint32_t>(max, advances[i]);
		if (max == advances[i])
			maxIdx = i;
	}

	// Remove or add from the max in case the sum still isn't 256
	if (sum > range)	advances[maxIdx] -= (sum - range);
	if (sum < range)	advances[maxIdx] += (range - sum);
}

namespace DStream
{
	template class StreamCoder<Hilbert>;
	template class StreamCoder<Morton>;
	template class StreamCoder<Hue>;
	template class StreamCoder<Phase>;
	template class StreamCoder<Triangle>;
	template class StreamCoder<Packed2>;
	template class StreamCoder<Split2>;
	template class StreamCoder<Split3>;
	template class StreamCoder<Packed3>;

	template <typename CoderImplementation>
	StreamCoder<CoderImplementation>::StreamCoder(bool enlarge, bool interpolate, uint8_t algoBits,
		std::vector<uint8_t> channelDistribution, bool useTables /* = true*/)
	{
		m_UseTables = useTables;
		m_Enlarge = enlarge;
		m_Interpolate = interpolate;

		m_Implementation = CoderImplementation(algoBits, channelDistribution);
		if (!m_Implementation.SupportsInterpolation())
			m_Interpolate = false;
		if (!m_Implementation.SupportsEnlarge())
			m_Enlarge = false;
		m_AlgoBits = m_Implementation.GetAlgoBits();
		m_EnlargeBits = 0;

		// Check if there's enough space in the RGB cube to enlarge
		int points = 1 << (m_AlgoBits * 3);
		int seg = 256 / ((1 << m_AlgoBits) - 1);
		int tot = points * seg;
		if (tot < 65535)
			m_Enlarge = false;

		// Generate tables
		if (m_Enlarge)
			GenerateSpacingTables();
		if (m_UseTables)
			GenerateCodingTables();
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::Encode(const uint16_t* source, Color* dest, uint32_t nElements)
	{
		uint32_t segmentSize = 256 / (1 << m_AlgoBits);
		if (m_UseTables)
		{
			for (uint32_t i = 0; i < nElements; i++)
				dest[i] = m_EncodingTable[source[i]];
		}
		else
			EncodeWithoutTables(dest, source, nElements);
	}


	// Interpolate values from the table if necessary
	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::Decode(const Color* source, uint16_t* dest, uint32_t nElements)
	{
		if (m_UseTables)
		{
			for (uint32_t i = 0; i < nElements; i++)
				dest[i] = m_DecodingTable[source[i][0]*256*256 + source[i][1]*256 + source[i][2]];
		}
		else
			DecodeWithoutTables(dest, source, nElements);
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::EncodeWithoutTables(Color* dest, const uint16_t* source, uint32_t nElements)
	{
		Color prev, curr;
		uint16_t nSegments = (1 << (m_AlgoBits * 3)) - 1;
		uint16_t maxVal = 0;

		for (uint32_t i = 0; i < nElements; i++)
		{
			if (m_Interpolate)
			{
				uint16_t startPoint, endPoint;
				uint32_t gridSide = (1 << m_AlgoBits) - 1;
				float currPoint = ((float)source[i] / 65535) * nSegments;
				float t = currPoint - std::floor(currPoint);

				// Find out where in the curve you are
				startPoint = std::floor(currPoint);
				endPoint = std::ceil(currPoint);

				// Encode those values
				Color startColor = m_Implementation.EncodeValue(startPoint);
				Color endColor = m_Implementation.EncodeValue(endPoint);

				// Map them between 0,256
				for (uint32_t j = 0; j < 3; j++)
				{
					startColor[j] = std::round(((float)startColor[j] / gridSide) * 255);
					endColor[j] = std::round(((float)endColor[j] / gridSide) * 255);
				}

				// Interpolate
				dest[i] = InterpolateColor(startColor, endColor, t);
			}
			else
			{
				if (std::is_same<Hilbert, CoderImplementation>())
					dest[i] = m_Implementation.EncodeValue(source[i] >> (16 - m_AlgoBits * 3));
				else
					dest[i] = m_Implementation.EncodeValue(source[i]);
			}
		}

		if (m_Enlarge)
			Enlarge(dest, dest, nElements);
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::DecodeWithoutTables(uint16_t* dest, const Color* source, uint32_t nElements)
	{
		Color* inCols = new Color[nElements];
		memcpy(inCols, source, 3 * nElements);// (Color*)source;
		if (m_Enlarge)
			Shrink(source, inCols, nElements);

		for (uint32_t i = 0; i < nElements; i++)
		{
			if (m_Interpolate)
				dest[i] = InterpolateHeight(inCols[i]);
			else if (std::is_same<Hilbert, CoderImplementation>())
				dest[i] = m_Implementation.DecodeValue(inCols[i]) << (16 - m_AlgoBits * 3);
			else
				dest[i] = m_Implementation.DecodeValue(inCols[i]);
		}
		delete[] inCols;
	}

	template<class CoderImplementation>
	Color StreamCoder<CoderImplementation>::InterpolateColor(const Color& a, const Color& b, float t)
	{
		Color ret;
		for (uint32_t i = 0; i < 3; i++)
			ret[i] = a[i] + std::round((b[i] - a[i]) * t);
		return ret;
	}

	template<class CoderImplementation>
	uint16_t StreamCoder<CoderImplementation>::InterpolateHeight(const Color& col)
	{
		uint32_t gridSide = (1 << m_AlgoBits) - 1;

		float Uf, Vf, Wf;
		float u = modf(((float)col[0] / 255.0f) * gridSide, &Uf);
		float v = modf(((float)col[1] / 255.0f) * gridSide, &Vf);
		float w = modf(((float)col[2] / 255.0f) * gridSide, &Wf);

		int uN = std::round(u), vN = std::round(v), wN = std::round(w);
		uint8_t U = (uint8_t)Uf, V = (uint8_t)Vf, W = (uint8_t)Wf;

		Color c000(U, V, W);
		Color c001(U, V, W + 1);
		Color c010(U, V + 1, W);
		Color c011(U, V + 1, W + 1);
		Color c100(U + 1, V, W);
		Color c101(U + 1, V, W + 1);
		Color c110(U + 1, V + 1, W);
		Color c111(U + 1, V + 1,  W + 1);

		/* Depth values
			  G----H
			 /|   /|
			C----D |
			| E--|-F
			A----B
		*/
		uint16_t A = m_Implementation.DecodeValue(c000), E = m_Implementation.DecodeValue(c001), C = m_Implementation.DecodeValue(c010),
			G = m_Implementation.DecodeValue(c011), B = m_Implementation.DecodeValue(c100), F = m_Implementation.DecodeValue(c101),
			D = m_Implementation.DecodeValue(c110), H = m_Implementation.DecodeValue(c111);
		uint16_t vals[] = { A,B,C,D,E,F,G,H };
		float interpVals[] = {
			(1 - u) * (1 - v) * (1 - w),
			u * (1 - v) * (1 - w),
			(1 - u) * v * (1 - w),
			u * v * (1 - w),
			(1 - u) * (1 - v) * w,
			u * (1 - v) * w,
			(1 - u) * v * w,
			u * v * w
		};
		float tot = 0;
		float T = vals[uN + vN * 2 + wN * 4];
		float threshold = 1 << m_AlgoBits;

		for (uint32_t i = 0; i < 2; i++)
		{
			for (uint32_t j = 0; j < 2; j++)
			{
				for (uint32_t k = 0; k < 2; k++)
				{
					uint32_t idx = i * 4 + j * 2 + k;
					if (std::abs((int)vals[idx] - T) > threshold)
						interpVals[idx] = 0;
					else
						tot += interpVals[idx];
				}
			}
		}

		float val = 0;
		for (uint32_t i = 0; i < 8; i++)
			val += interpVals[i] * vals[i] / tot;

		return std::round((val / ((1 << (m_AlgoBits * 3)) - 1)) * 65535);
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::GenerateCodingTables()
	{
		uint32_t maxQuantizationValue = (1 << 16);
		uint32_t maxAlgoBitsValue = (1 << 8);

		m_DecodingTable.resize(maxAlgoBitsValue * maxAlgoBitsValue * maxAlgoBitsValue);
		for (uint32_t i = 0; i < maxAlgoBitsValue; i++)
		{
			for (uint32_t j = 0; j < maxAlgoBitsValue; j++)
			{
				for (uint32_t k = 0; k < maxAlgoBitsValue; k++)
				{
					Color c = { (uint8_t)i, (uint8_t)j, (uint8_t)k };
					uint16_t val;

					DecodeWithoutTables(&val, &c, 1);
					m_DecodingTable[i * maxAlgoBitsValue * maxAlgoBitsValue + j * maxAlgoBitsValue + k] = val;
				}
			}
		}

		m_EncodingTable.resize(maxQuantizationValue);
		Color c;
		for (uint32_t i = 0; i < maxQuantizationValue; i++)
		{
			uint16_t val = i;
			EncodeWithoutTables(&c, &val, 1);
			m_EncodingTable[i] = c;
		}
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::GenerateSpacingTables()
	{
		// Init tables
		uint32_t side = 1 << m_AlgoBits;
		// Init table memory
		uint16_t* table = new uint16_t[side * side * side];

		for (uint16_t i = 0; i < side; i++)
		{
			for (uint16_t j = 0; j < side; j++)
			{
				for (uint16_t k = 0; k < side; k++)
				{
					uint16_t val = m_Implementation.DecodeValue(Color((uint8_t)i, (uint8_t)j, (uint8_t)k)) / (1 << (16 - m_AlgoBits*3));
					table[i * side * side + j * side + k] = val;
				}
			}
		}

		// Compute spacing tables
		for (uint32_t e = 0; e < 3; e++)
		{
			// Init error vector
			std::vector<uint16_t> errors = GetErrorVector(table, side, e);
			uint32_t minAdvanceSize = 65535 / ((1 << (m_AlgoBits * 3)) - 1);

			// Apply multiple times to ensure it's normaliozed
			bool normalized = false;
			do
			{
				NormalizeAdvance(errors, 256, minAdvanceSize);
				normalized = true;
				for (uint32_t i = 0; i < errors.size() && normalized; i++)
					if (errors[i] < minAdvanceSize)
						normalized = false;
			} while (!normalized);
			
			float sum = 0;
			float errSum = 0;
			float fSegSize = std::ceil((float)255 / ((1 << m_AlgoBits) - 1));
			int segSize = fSegSize;
			uint32_t errIndex = 0;

			m_SpacingTable.Shrink[e].resize(256);
			for (uint32_t i = 0; i < 256; i++)
			{
				if (errSum >= errors[errIndex])
				{
					errSum = 0;
					errIndex++;
				}

				uint32_t shrinkIdx = (int)std::round(sum);

				m_SpacingTable.Enlarge[e].push_back(shrinkIdx);
				if (m_SpacingTable.Shrink[e][shrinkIdx] == 0)
					m_SpacingTable.Shrink[e][shrinkIdx] = i;

				float increase = (float)errors[errIndex] / fSegSize;
				sum += increase;
				errSum += increase;
			}

			// Fill remaining gaps
			for (uint32_t i = 1; i < 256; i++)
			{
				if (m_SpacingTable.Shrink[e][i] == 0)
					m_SpacingTable.Shrink[e][i] = m_SpacingTable.Shrink[e][i - 1];

				if (m_SpacingTable.Enlarge[e][i] == 0)
					m_SpacingTable.Enlarge[e][i] = m_SpacingTable.Enlarge[e][i - 1];
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
	std::vector<uint16_t> StreamCoder<CoderImplementation>::GetErrorVector(uint16_t* table, uint32_t tableSide, uint8_t axis, uint8_t amount)
	{
		uint32_t errSize = tableSide - 1;
		std::vector<uint16_t> ret(tableSide-1, 0);

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
					default:
						tableLeft = 1;
						tableRight = 0;
						break;
					}

					ret[k] = std::max<int>(ret[k], std::abs(tableLeft - tableRight));
				}
			}
		}

		return ret;
	}
}
