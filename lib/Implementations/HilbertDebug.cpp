#include <Implementations/HilbertDebug.h>

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

    HilbertDebug::HilbertDebug(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistributions) : Coder(quantization, algoBits, channelDistributions)
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
        
        m_Morton = Morton(quantization, m_AlgoBits, { 8,8,8 }, true);
	}

	Color HilbertDebug::EncodeValue(uint16_t val)
	{
		Color v = m_Morton.EncodeValue(val);
		std::swap(v[0], v[2]);
		TransposeFromHilbertCoords(v);

        return v;
	}

    uint16_t HilbertDebug::DecodeValue(Color col1)
    {
        Color currColor = col1;
        TransposeToHilbertCoords(currColor);
        std::swap(currColor[0], currColor[2]);
        uint16_t v1 = m_Morton.DecodeValue(currColor);
        
        return v1;
    }


    void HilbertDebug::TransposeFromHilbertCoords(Color& col)
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

    void HilbertDebug::TransposeToHilbertCoords(Color& col)
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