#include <StreamCoder.h>
#include <Implementations/Hilbert.h>
/*
#include <Implementations/HilbertDebug.h>
#include <Implementations/Hue.h>
#include <Implementations/Packed2.h>
#include <Implementations/Packed3.h>
#include <Implementations/Split2.h>
#include <Implementations/Split3.h>
#include <Implementations/Phase.h>
#include <Implementations/Triangle.h>
*/

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
	/*
	template class StreamCoder<HilbertDebug>;
	template class StreamCoder<Morton>;
	template class StreamCoder<Split2>;
	template class StreamCoder<Split3>;
	template class StreamCoder<Packed2>;
	template class StreamCoder<Packed3>;
	template class StreamCoder<Phase>;
	template class StreamCoder<Hue>;
	template class StreamCoder<Triangle>;
	*/

	template <typename CoderImplementation>
	StreamCoder<CoderImplementation>::StreamCoder(bool enlarge, bool interpolate, uint8_t algoBits,
		std::vector<uint8_t> channelDistribution, bool useTables /* = true*/)
	{
		m_UseTables = useTables;
		m_Enlarge = enlarge;
		m_Interpolate = interpolate;

		m_Implementation = CoderImplementation(16, algoBits, channelDistribution);
		m_AlgoBits = m_Implementation.GetAlgoBits();
		m_EnlargeBits = 0;

		if (enlarge)
			GenerateSpacingTables();
		if (useTables)
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
		{
			Color prev, curr;
			uint16_t nSegments = (1 << (m_AlgoBits * 3)) - 1;
			uint16_t maxVal = 0;

			for (uint32_t i = 0; i < nElements; i++)
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

			/*
			if (m_Enlarge)
			{
				Color* enlarged = new Color[nElements];
				Enlarge(dest, enlarged, nElements);
				memcpy(dest, enlarged, nElements * 3);
				delete[] enlarged;
			}
			*/
		}
	}


	// Interpolate values from the table if necessary
	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::Decode(const Color* source, uint16_t* dest, uint32_t nElements)
	{
		Color* inCols = new Color[nElements];
		memcpy(inCols, source, 3 * nElements);// (Color*)source;
		/*
		if (m_Enlarge)
		{
			inCols = new Color[nElements];
			Shrink(source, inCols, nElements);
		}
		*/

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
			{
				if (m_Interpolate)
					dest[i] = InterpolateHeight(inCols[i]);
				else
					dest[i] = m_Implementation.DecodeValue(inCols[i]);
			}
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
	uint16_t StreamCoder<CoderImplementation>::InterpolateHeight(const Color& c)
	{
		Color c1 = {c.x, c.y, c.z};
		uint32_t gridSide = (1 << m_AlgoBits) - 1;
		for (uint32_t i = 0; i < 3; i++)
			c1[i] = std::round(((float)c[i] / 255.0f) * gridSide);

		float Uf, Vf, Wf;
		float u = modf(((float)c[0] / 255.0f) * gridSide, &Uf);
		float v = modf(((float)c[1] / 255.0f) * gridSide, &Vf);
		float w = modf(((float)c[2] / 255.0f) * gridSide, &Wf);

		uint8_t U = (uint8_t)Uf, V = (uint8_t)Vf, W = (uint8_t)Wf;

		// BACK COLORS
		Color c111(U + 1,    V + 1,  W + 1);
		Color c011(U,        V + 1,  W + 1);
		Color c101(U + 1,    V,      W + 1);
		Color c001(U,        V,      W + 1);

		// FRONT COLORS
		Color c110(U + 1,    V + 1,  W);
		Color c010(U,        V + 1,  W);
		Color c100(U + 1,    V,      W);
		Color c000(U,        V,      W);

		/* Depth values
			  G---H
			C---D |
			| E-|-F
			A---B
		*/
		uint16_t A = m_Implementation.DecodeValue(c000), E = m_Implementation.DecodeValue(c001), C = m_Implementation.DecodeValue(c010),
			G = m_Implementation.DecodeValue(c011), B = m_Implementation.DecodeValue(c100), F = m_Implementation.DecodeValue(c101),
			D = m_Implementation.DecodeValue(c110), H = m_Implementation.DecodeValue(c111);

		// Interpolation values
		float threshold = 1 << m_AlgoBits;

		float uAB = std::abs(A - B) > threshold ? std::round(u) : u;
		float uCD = std::abs(C - D) > threshold ? std::round(u) : u;
		float uEF = std::abs(E - F) > threshold ? std::round(u) : u;
		float uGH = std::abs(G - H) > threshold ? std::round(u) : u;

		float vAC = std::abs(A - C) > threshold ? std::round(v) : v;
		float vBD = std::abs(B - D) > threshold ? std::round(v) : v;
		float vEG = std::abs(E - G) > threshold ? std::round(v) : v;
		float vFH = std::abs(F - H) > threshold ? std::round(v) : v;

		float wAE = std::abs(A - E) > threshold ? std::round(w) : w;
		float wBF = std::abs(B - F) > threshold ? std::round(w) : w;
		float wCG = std::abs(C - G) > threshold ? std::round(w) : w;
		float wDH = std::abs(D - H) > threshold ? std::round(w) : w;

		// Interpolate values
		float val = A * (1 - uAB) * (1 - vAC) * (1 - wAE) +
			B * uAB * (1 - vBD) * (1 - wBF) +
			C * (1 - uCD) * vAC * (1 - wCG) +
			D * uCD * vBD * (1 - wDH) +

			E * (1 - uEF) * (1 - vEG) * wAE +
			F * uEF * (1 - vFH) * wBF +
			G * (1 - uGH) * vEG * wCG +
			H * uGH * vFH * wDH;

		return std::round((val / ((1 << (m_AlgoBits * 3)) - 1)) * 65535);
	}

	template<class CoderImplementation>
	void StreamCoder<CoderImplementation>::GenerateCodingTables()
	{
		uint32_t maxQuantizationValue = (1 << 16);
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
