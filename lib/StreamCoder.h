#pragma once

#include <cstdint>
#include <unordered_map>
#include <type_traits>

#include <Coder.h>
#include <DataStructs/Table.h>
#include <DataStructs/Vec3.h>

namespace DStream
{
	template <typename CoderImplementation>
	class StreamCoder
	{
		static_assert(std::is_base_of_v<Coder, CoderImplementation>, "Template parameter of class StreamCoder must derive from Coder.");
	public:
		StreamCoder(bool enlarge, bool interpolate, uint8_t algoBits, std::vector<uint8_t> channelDistribution, bool useTables = true);
		StreamCoder() = default;
		~StreamCoder() = default;

		void Encode(const uint16_t* source, Color* dest, uint32_t nElements);
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
			for (uint32_t i=0; i<nElements; i++)
				for (int k = 0; k < 3; k++)
					dest[i][k] = m_SpacingTable.Enlarge[k][source[i][k]];		
		}

		inline void Shrink(const Color* source, Color* dest, uint32_t nElements)
		{
			for (uint32_t i = 0; i < nElements; i++)
				for (int k = 0; k < 3; k++)
					dest[i][k] = m_SpacingTable.Shrink[k][source[i][k]];
		}

	CoderImplementation m_Implementation;

	private:
		std::vector<uint16_t> GetErrorVector(uint16_t* decodingTable, uint32_t tableSide, uint8_t axis, uint8_t amount = 1);

		void DecodeWithoutTables(uint16_t* dest, const Color* source, uint32_t nElements);
		void EncodeWithoutTables(Color* dest, const uint16_t* source, uint32_t nElements);

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