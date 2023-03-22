#include <Implementations/Hilbert2.h>

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

    Hilbert2::Hilbert2(uint8_t quantization, uint8_t algoBits, std::vector<uint8_t> channelDistributions) : Coder(quantization, algoBits, channelDistributions)
	{
#ifdef _WIN32
		assert(algoBits * 2 < quantization);
#else
		assert(algoBits * 2 < quantization);
#endif

		m_AlgoBits = algoBits;
		m_SegmentBits = quantization - 2 * m_AlgoBits;

#ifdef _WIN32
		assert(m_AlgoBits + m_SegmentBits <= 8);
#else
		assert(m_AlgoBits + m_SegmentBits <= 8);
#endif
	}

    Color Hilbert2::Encode2D(uint16_t val)
    {
        uint8_t x = 0, y = 0, tmp;
        uint32_t n = 1 << m_AlgoBits;
        for (uint32_t s = 1; s < n; s *= 2) {
            uint32_t rx = 1 & (val / 2);
            uint32_t ry = 1 & (val ^ rx);
            // inlining
            // hilbertRot(s, p, rx, ry)
            if (ry == 0) {
                if (rx == 1) {
                    x = s - 1 - x;
                    y = s - 1 - y;
                }
                tmp = x;
                x = y;
                y = tmp;
            }
            // end inline

            x += s * rx;
            y += s * ry;
            val /= 4;
        }
        return { x, y, 0 };
    }

    uint16_t Hilbert2::Decode2D(Color c)
    {
        uint32_t d = 0, tmp;
        uint32_t x = c[0], y = c[1];
        for (uint32_t s = (1 << m_AlgoBits) / 2; s > 0; s /= 2) {
            uint32_t rx = 0, ry = 0;

            if ((x & s) > 0) rx = 1;
            if ((y & s) > 0) ry = 1;

            d += s * s * ((3 * rx) ^ ry);

            // inlining
            // hilbertRot(s, p, rx, ry)
            if (ry == 0) {
                if (rx == 1) {
                    x = s - 1 - x;
                    y = s - 1 - y;
                }
                tmp = x;
                x = y;
                y = tmp;
            }
            // end inline
        }
        return d;
    }

	Color Hilbert2::EncodeValue(uint16_t val)
	{
		int frac = val & ((1 << m_SegmentBits) - 1);
		val >>= m_SegmentBits;

		Color v = Encode2D(val);

        assert(val < (1 << (m_AlgoBits * 2)));

        if (val + 1 != (1 << (m_AlgoBits * 2)))
        {
            // SUBDIVISION
            Color v2 = Encode2D(val + 1);

            // Divide in segments
            for (uint32_t i = 0; i < 2; i++)
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
            for (uint32_t i = 0; i < 2; i++)
                v[i] = (v[i] << m_SegmentBits);
            v[0] += frac;
        }

        return v;
	}

    uint16_t Hilbert2::DecodeValue(Color col1)
    {
        int side = 1 << m_SegmentBits;
        Color currColor;

        int fract[2];
        uint32_t maxVal = (1 << (m_AlgoBits + m_SegmentBits)) - 1;

        for (uint32_t i = 0; i < 2; i++)
        {
            fract[i] = col1[i] & ((1 << m_SegmentBits) - 1);
            col1[i] >>= m_SegmentBits;
        }

        currColor = col1;
        uint16_t v1 = Decode2D(col1);
        
        if (v1+1 != (1 << (m_AlgoBits * 2)))
        {
            uint16_t v2 = v1 + 1;
            Color nextCol = Encode2D(v2);

            uint16_t v3 = std::max<int>((int)v1 - 1, 0);
            Color prevCol = Encode2D(v3);

            int sign = 1;
            for (uint32_t i = 0; i < 2 && sign == 1; i++)
                if (nextCol[i] != prevCol[i] && fract[i] != 0)
                    sign *= nextCol[i] - prevCol[i];

            if (sign < 0)
            {
                nextCol = prevCol;
                prevCol = currColor;
                std::swap(nextCol, prevCol);
            }

            v1 <<= m_SegmentBits;
            for (int i = 0; i < 2; i++)
                v1 += fract[i] * sgn(nextCol[i] - prevCol[i]);
        }
        else
        {
            uint16_t v3 = v1 - 1;
            Color prevCol = Encode2D(v3);

            int sign = 1;
            for (uint32_t i = 0; i < 2 && sign == 1; i++)
                if (currColor[i] != prevCol[i] && fract[i] != 0)
                {
                    sign *= currColor[i] - prevCol[i];
                    if (sign < 0)
                        fract[i] = side - fract[i];
                }

            if (sign < 0)
            {
                v3 <<= m_SegmentBits;
                for (int i = 0; i < 2; i++)
                    v3 += fract[i] * sgn(prevCol[i] - currColor[i]);
                return v3;
            }

            v1 <<= m_SegmentBits;
            v1 += fract[0];
        }
       
        return v1;
    }


    void Hilbert2::TransposeFromHilbertCoords(Color& col)
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

    void Hilbert2::TransposeToHilbertCoords(Color& col)
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