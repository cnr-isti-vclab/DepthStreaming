#include <Implementations/Hilbert.h>

#include <utility>
#include <cassert>
#include <iostream>

// Credits for basic Hilbert convertions: https://github.com/davemc0/DMcTools/blob/main/Math/SpaceFillCurve.h

namespace DStream
{
    template <typename T>
    static inline int sgn(T val)
    {
        return (T(0) < val) - (val < T(0));
    }

    Hilbert::Hilbert(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistributions) : Coder(quantization, algoBits, channelDistributions)
	{
#ifdef _WIN32
		assert(algoBits * 3 < quantization);
#else
		assert(algoBits * 3 < quantization);
#endif

		m_AlgoBits = algoBits;
		m_SegmentBits = quantization - 3 * m_AlgoBits;

#ifdef _WIN32
		assert(m_AlgoBits + m_SegmentBits <= 8);
#else
		assert(m_AlgoBits + m_SegmentBits <= 8);
#endif
        
        m_Morton = Morton(quantization, algoBits, { 8,8,8 }, true);
	}

	Color Hilbert::EncodeValue(uint16_t val)
	{
		int frac = val & ((1 << m_SegmentBits) - 1);
		val >>= m_SegmentBits;

		Color v = m_Morton.EncodeValue(val);
		std::swap(v[0], v[2]);
		TransposeFromHilbertCoords(v);

        // Assert che sia <=
        if (val + 1 != (1 << (m_AlgoBits * 3)))
        {
            // SUBDIVISION
            Color v2 = m_Morton.EncodeValue(val + 1);
            std::swap(v2[0], v2[2]);
            TransposeFromHilbertCoords(v2);

            // Divide in segments
            for (uint32_t i = 0; i < 3; i++)
            {
                uint8_t side = 1 << m_SegmentBits;
                uint8_t mult = sgn(v2[i] - v[i]) * frac;
                uint8_t overflown = mult + side;

                v[i] <<= m_SegmentBits;

                if (overflown < mult)
                    v[i] -= side - overflown;
                else
                    v[i] += mult;
            }
        }
        else
        {
            for (uint32_t i = 0; i < 3; i++)
                v[i] = (v[i] << m_SegmentBits);
            v[0] += frac;
        }

        return v;
	}

    uint16_t Hilbert::DecodeValue(Color col1)
    {
        int side = 1 << m_SegmentBits;
        Color currColor;

        int fract[3];
        uint32_t maxVal = (1 << (m_AlgoBits + m_SegmentBits)) - 1;

        for (uint32_t i = 0; i < 3; i++)
        {
            fract[i] = col1[i] & ((1 << m_SegmentBits) - 1);
            col1[i] >>= m_SegmentBits;
        }

        currColor = col1;
        TransposeToHilbertCoords(col1);
        std::swap(col1[0], col1[2]);
        uint16_t v1 = m_Morton.DecodeValue(col1);
        
        if (v1+1 != (1 << (m_AlgoBits * 3)))
        {
            uint16_t v2 = v1 + 1;
            Color nextCol = m_Morton.EncodeValue(v2);
            std::swap(nextCol[0], nextCol[2]);
            TransposeFromHilbertCoords(nextCol);

            uint16_t v3 = std::max<int>((int)v1 - 1, 0);
            Color prevCol = m_Morton.EncodeValue(v3);
            std::swap(prevCol[0], prevCol[2]);
            TransposeFromHilbertCoords(prevCol);

            int sign = 1;
            for (uint32_t i = 0; i < 3 && sign == 1; i++)
                if (nextCol[i] != prevCol[i] && fract[i] != 0)
                    sign *= nextCol[i] - prevCol[i];

            if (sign < 0)
            {
                nextCol = prevCol;
                prevCol = currColor;
                std::swap(nextCol, prevCol);
            }

            v1 <<= m_SegmentBits;
            for (int i = 0; i < 3; i++)
                v1 += fract[i] * sgn(nextCol[i] - prevCol[i]);
        }
        else
        {
            uint16_t v3 = v1 - 1;
            Color prevCol = m_Morton.EncodeValue(v3);
            std::swap(prevCol[0], prevCol[2]);
            TransposeFromHilbertCoords(prevCol);

            int sign = 1;
            for (uint32_t i = 0; i < 3 && sign == 1; i++)
                if (currColor[i] != prevCol[i] && fract[i] != 0)
                {
                    sign *= currColor[i] - prevCol[i];
                    fract[i] = side - fract[i];
                }

            if (sign < 0)
            {
                v3 <<= m_SegmentBits;
                for (int i = 0; i < 3; i++)
                    v3 += fract[i] * sgn(prevCol[i] - currColor[i]);
                return v3;
            }

            v1 <<= m_SegmentBits;
            v1 += fract[0];
        }
       
        return v1;
    }


    void Hilbert::TransposeFromHilbertCoords(Color& col)
    {
        int X[3] = { col.x, col.y, col.z };
        uint32_t N = 2 << (m_AlgoBits - 1), P, Q, t;

        // Gray decode by H ^ (H/2)
        t = X[3 - 1] >> 1;
        // Corrected error in Skilling's paper on the following line. The appendix had i >= 0 leading to negative array index.
        for (int i = 3 - 1; i > 0; i--) X[i] ^= X[i - 1];
        X[0] ^= t;

        // Undo excess work
        for (Q = 2; Q != N; Q <<= 1) {
            P = Q - 1;
            for (int i = 3 - 1; i >= 0; i--)
                if (X[i] & Q) // Invert
                    X[0] ^= P;
                else { // Exchange
                    t = (X[0] ^ X[i]) & P;
                    X[0] ^= t;
                    X[i] ^= t;
                }
        }

        col.x = X[0]; col.y = X[1]; col.z = X[2];
    }

    void Hilbert::TransposeToHilbertCoords(Color& col)
    {
        int X[3] = { col.x, col.y, col.z };
        uint32_t M = 1 << (m_AlgoBits - 1), P, Q, t;

        // Inverse undo

        for (Q = M; Q > 1; Q >>= 1) {
            P = Q - 1;
            for (int i = 0; i < 3; i++) {
                if (X[i] & Q) // Invert
                    X[0] ^= P;
                else { // Exchange
                    t = (X[0] ^ X[i]) & P;
                    X[0] ^= t;
                    X[i] ^= t;
                }
            }
        }

        // Gray encode
        for (int i = 1; i < 3; i++) X[i] ^= X[i - 1];
        t = 0;
        for (Q = M; Q > 1; Q >>= 1)
            if (X[3 - 1] & Q) t ^= Q - 1;
        for (int i = 0; i < 3; i++) X[i] ^= t;

        col.x = X[0]; col.y = X[1]; col.z = X[2];
    }
}