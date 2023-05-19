#pragma once

#include <cstdint>
#include <unordered_map>
#include <type_traits>

#include <Coder.h>
#include <DataStructs/Table.h>
#include <DataStructs/Vec3.h>

namespace DStream
{
	// [TMP]
	struct AxisErrors
	{
		float AvgErr = 0;
		float MaxErr = 0;
		float QuadErr = 0;
	};


	template <	typename CoderImplementation
				/*,typename std::enable_if<std::is_base_of_v<Coder, CoderImplementation>, bool>::type = true*/>
	class StreamCoder
	{
		static_assert(std::is_base_of_v<Coder, CoderImplementation>, "Template parameter of class StreamCoder must derive from Coder.");
	public:
		StreamCoder(bool enlarge, bool interpolate, uint8_t algoBits, std::vector<uint8_t> channelDistribution, bool useTables = true);
		StreamCoder() = default;
		~StreamCoder() = default;

		void Encode(const uint16_t* source, Color* dest, uint32_t nElements);
		// Interpolate values from the table if necessary
		void Decode(const Color* source, uint16_t* dest, uint32_t nElements);

		void GenerateCodingTables();
		void GenerateSpacingTables();
		uint16_t* GetDecodingTable() { return m_DecodingTable.data(); }

		void SetSpacingTables(SpacingTable tables);
		void SetEncodingTable(const std::vector<Color>& table);
		void SetDecodingTable(const std::vector<uint16_t>& table, uint32_t tableSideX, uint32_t tableSideY, uint32_t tableSideZ);

		inline Color InterpolateColor(const Color& a, const Color& b, float t);
		uint16_t InterpolateHeight(const Color& c);

		inline void Enlarge(const Color* source, Color* dest, uint32_t nElements)
		{
			uint8_t bits = 8 - m_Implementation.GetEnlargeBits();
			for (uint32_t i = 0; i < nElements; i++)
				for (int k = 0; k < 3; k++)
					dest[i][k] = source[i][k] << (8 - m_Implementation.GetEnlargeBits());

					/*
			for (uint32_t i=0; i<nElements; i++)
				for (int k = 0; k < 3; k++)
					dest[i][k] = m_SpacingTable.Enlarge[k][source[i][k]];		
					*/
		}

		inline void Shrink(const Color* source, Color* dest, uint32_t nElements)
		{
			for (uint32_t i = 0; i < nElements; i++)
				for (int k = 0; k < 3; k++)
					dest[i][k]  = source[i][k] >> (8 - m_Implementation.GetEnlargeBits());

					/*
			for (uint32_t i = 0; i < nElements; i++)
				for (int k = 0; k < 3; k++)
				{
					dest[i][k] = m_SpacingTable.Shrink[k][source[i][k]];
					if (std::is_same<CoderImplementation, Hilbert>())
						dest[i][k] <<= (8 - m_Implementation.GetEnlargeBits());
				}
				*/
		}

	CoderImplementation m_Implementation;

	private:
		// [TMP]
		std::vector<uint16_t> GetErrorVector(uint16_t* decodingTable, uint32_t tableSide, uint8_t axis, AxisErrors& ret, uint8_t amount = 1);

	private:
		bool m_UseTables;
		bool m_Enlarge;
		bool m_Interpolate;

		uint32_t m_AlgoBits;
		uint32_t m_EnlargeBits;
		
		SpacingTable m_SpacingTable;
		std::vector<Color> m_EncodingTable;
		std::vector<uint16_t> m_DecodingTable;
	};

}